# webserv

НУ ЧЕ НА-НАРОД ПОГНАЛИ НАХУЙ ЕБАНЫЙРООООООООООООТ

Liza's shit


Resources: 

YT vids: https://www.youtube.com/watch?v=9J1nJOivdyw
https://www.youtube.com/watch?v=iInUBOVeBCc&t=684s - wtf is NGINX
https://www.youtube.com/watch?v=2CjQbMGYNIk&t=454s - non-blocking IO (sockets)


A webserver listens on one or more ports (as a rule different processes cannot listen on the same port (linux has exceptions to that?? but nvm)) for a request sent via some transport protocol and returns a response containing the requested resource (a webpage or w/e idek)
- Determining which resource to return is called routing (page/text/image/YT video/etc) - basically connecting the request with the requested resource
    - Static routing: serve files from disk. A request for /images/cat.png gets mapped to a directory you set in the config, and you send the file back (ok sounds ez)
    - Dynamic: CGI. When a request matches a configured extension (like .php or .py), you run that script with fork and execve, pass it the request, and send its output back as the response (ok not so ez)


Http protocol:

> GET / HTTP/1.1
> Host: localhost
> User-Agent: curl/8.5.0
> Accept: */*


TO FIGURE OUT:
???blocking/non-blocking syscalls
???asynchronous programming 
???run a script on my server
???config NGINX

TODO
FORKING FOR CGI WHERE HOW

inline fc's figure out

start Makefile!!

c++  -Iinclude -Ienum main.cpp src/http/*.cpp -o webserv -std=c++20

We close the client socket not after each event but after the full response has been sent!!! which kinda same in my mind but ok

"Close only when the response is completely written, or on error, timeout, or when recv() returns 0 (client hung up)."

"Send a correct Content-Length and Connection: close, then close after the response is fully flushed. That’s simple and correct. Supporting keep-alive is a nice extra, and it helps with stress testers like siege that reuse connections, but it isn’t required for HTTP/1.0 behavior."
