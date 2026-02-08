#include "mongoose.h"
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <unistd.h>
#if defined(__APPLE__) || defined(__FreeBSD__)
#include <util.h>
#else
#include <pty.h>
#endif
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>

// Connection state
struct ConnectionState {
  int pty_fd;
  pid_t pid;
  struct mg_connection *c;
};

// Global map to track connections
std::map<struct mg_connection *, ConnectionState> connections;

// Function to read from file
std::string read_file(const std::string &path) {
  std::ifstream file(path);
  if (!file.is_open())
    return "";
  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

// Event handler for Mongoose
static void fn(struct mg_connection *c, int ev, void *ev_data) {
  if (ev == MG_EV_HTTP_MSG) {
    struct mg_http_message *hm = (struct mg_http_message *)ev_data;
    if (mg_match(hm->uri, mg_str("/ws"), NULL)) {
      mg_ws_upgrade(c, hm, NULL);
    } else if (mg_match(hm->uri, mg_str("/terminal"), NULL)) {
      // Serve terminal app
      std::string content = read_file("terminal.html");
      if (content.empty()) {
        mg_http_reply(c, 404, "", "Terminal file not found\n");
      } else {
        mg_http_reply(c, 200, "Content-Type: text/html\r\n", "%s",
                      content.c_str());
      }
    } else {
      // Serve Index (Landing Page)
      std::string content = read_file("index.html");
      if (content.empty()) {
        mg_http_reply(c, 404, "", "Index file not found\n");
      } else {
        mg_http_reply(c, 200, "Content-Type: text/html\r\n", "%s",
                      content.c_str());
      }
    }
  } else if (ev == MG_EV_WS_OPEN) {
    // Fork PTY
    int master_fd;
    pid_t pid = forkpty(&master_fd, NULL, NULL, NULL);

    if (pid == -1) {
      std::cerr << "Forkpty failed" << std::endl;
      return;
    }

    if (pid == 0) {
      // Child process
      setenv("TERM", "xterm-256color", 1);
      execl("/bin/zsh", "zsh", NULL);
      exit(1);
    }

    // Parent process: Set non-blocking
    int flags = fcntl(master_fd, F_GETFL, 0);
    fcntl(master_fd, F_SETFL, flags | O_NONBLOCK);

    // Store connection state
    connections[c] = {master_fd, pid, c};
    std::cout << "Client connected (PID: " << pid << ")" << std::endl;

  } else if (ev == MG_EV_WS_MSG) {
    struct mg_ws_message *wm = (struct mg_ws_message *)ev_data;
    // Write message to PTY
    if (connections.find(c) != connections.end()) {
      write(connections[c].pty_fd, wm->data.buf, wm->data.len);
    }
  } else if (ev == MG_EV_CLOSE) {
    if (connections.find(c) != connections.end()) {
      ConnectionState &state = connections[c];
      close(state.pty_fd);
      kill(state.pid, SIGKILL);
      waitpid(state.pid, NULL, 0);
      std::cout << "Client disconnected (PID: " << state.pid << ")"
                << std::endl;
      connections.erase(c);
    }
  }
}

int main() {
  struct mg_mgr mgr;
  mg_mgr_init(&mgr);
  mg_http_listen(&mgr, "http://0.0.0.0:8000", fn, NULL);

  std::cout << "Starting C++ Web Terminal Backend..." << std::endl;
  std::cout << "Serving at http://localhost:8000" << std::endl;

  // Launch browser automatically
  system("open http://localhost:8000");

  while (true) {
    mg_mgr_poll(&mgr, 10); // Poll every 10ms

    // Check PTYs for output
    char buf[4096];
    for (auto &pair : connections) {
      ConnectionState &state = pair.second;
      ssize_t n = read(state.pty_fd, buf, sizeof(buf));
      if (n > 0) {
        mg_ws_send(state.c, buf, n, WEBSOCKET_OP_TEXT);
      } else if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
        // Error reading PTY (maybe closed?)
        // Handle close logic if needed, but let client close normally
      }
    }
  }

  mg_mgr_free(&mgr);
  return 0;
}
