# webserv
погнали


Resources: 

YT vids: https://www.youtube.com/watch?v=9J1nJOivdyw

A webserver listens on one or more ports (as a rule different processes cannot listen on the same port (linux has exceptions to that?? but nvm)) for a request sent via some transport protocol and returns a response containing the requested resource (a webpage or w/e idek)
- Determining which resource to return is called routing (page/text/image/YT video/etc) - basically connecting the request with the requested resource
    - Static routing: serve files from disk. A request for /images/cat.png gets mapped to a directory you set in the config, and you send the file back (ok sounds ez)
    - Dynamic: CGI. When a request matches a configured extension (like .php or .py), you run that script with fork and execve, pass it the request, and send its output back as the response (wtf is CGI figure out!!!)


Http protocol:

> GET / HTTP/1.1
> Host: localhost
> User-Agent: curl/8.5.0
> Accept: */*
