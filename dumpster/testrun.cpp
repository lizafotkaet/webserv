/*
	tryna run .py in a separate process and pass it back through pipes smh
*/

#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <iostream>

#define NDEBUG
// std::string runChild(char** argv){
	

// }

int main(){
	char *argv[] = {(char*)"/usr/bin/python3", (char*)"scripts/test.py", NULL};
	int out[2];

	if (pipe(out) == -1)
        return (-1);
	
	
	std::cout << out[0] << " " << out[1] << '\n';
    pid_t pid = fork();
    if (pid == -1)
    {
        close(out[0]);
        close(out[1]);
        return (-1);
    }
	if (pid == 0) {
		dup2(out[1], STDOUT_FILENO);
		close(out[0]);
		close(out[1]);
		execve(argv[0], argv, NULL);
		_exit(1);
	}
	// int i = getpid();
	// std::cout << i << '\n';
	close(out[1]);
	std::string final;
	char buf[4096] = {};
	// in webserv: only after poll says readable
	ssize_t n;
	while ((n = read(out[0], buf, sizeof buf - 1)) > 0){
		final.append(buf, n);
	}
	// std::cout << "Output from childprocess: \n\n" << final << "\n";
	// if (n > 0) buf[n] = '\0';
	close(out[0]);
	int status;
	waitpid(pid, &status, 0);

	return 0;
}

/*
client request comes in for a .py
  → pipe(s), fork, execve           (fork doesn't block, fine to do inline)
  → set the parent's pipe ends O_NONBLOCK with fcntl
  → add the read end (stdout pipe) to poll with POLLIN
  → if POST: add the write end (stdin pipe) with POLLOUT
  → store a CGI state struct: pid, fds, client it belongs to, output buffer, start time

each poll iteration:
  stdin pipe POLLOUT → write() ONE chunk of the request body; when done, close it
  stdout pipe POLLIN → read() ONE chunk, append to buffer
  read() returns 0 (EOF) → close fd, waitpid(pid, &status, WNOHANG), check status,
                           parse the CGI output into a response, mark client POLLOUT
  started too long ago   → kill(pid, SIGKILL), waitpid, send 504


*/

/*

1. Pick the server block   → by the port it came in on (+ Host header if several servers share a port)
2. Pick the location       → longest prefix match of the URL path
3. Check the location's rules, in order:
     redirect set?                 → 301/302 with Location header, done
     method not allowed?           → 405
     body > client_max_body_size?  → 413
4. Map URL → filesystem path using root
5. Decide the handler:
     extension matches a cgi rule  → startCgi(interpreter, path)
     POST + upload allowed         → save file
     DELETE                        → unlink file
     path is a directory           → index file if it exists, else autoindex listing, else 403
     path is a file                → serve it
     doesn't exist                 → 404


*/