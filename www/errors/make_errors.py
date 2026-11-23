import sys, os
out = sys.argv[1]
codes = {
 400:("Bad Request","The server could not understand your request. Check the request line and headers."),
 403:("Forbidden","You don't have permission to access this resource."),
 404:("Not Found","The requested resource could not be found on this server."),
 405:("Method Not Allowed","This method is not allowed for the requested resource."),
 408:("Request Timeout","The server timed out waiting for the request."),
 411:("Length Required","This request needs a Content-Length header."),
 413:("Payload Too Large","The request body exceeds the configured client_max_body_size."),
 414:("URI Too Long","The requested URI is longer than the server is willing to interpret."),
 415:("Unsupported Media Type","The server does not support the media type of the request body."),
 500:("Internal Server Error","The server ran into an unexpected condition."),
 501:("Not Implemented","The server does not support the functionality required to fulfil this request."),
 502:("Bad Gateway","The CGI script returned an invalid response."),
 503:("Service Unavailable","The server is temporarily unable to handle the request."),
 504:("Gateway Timeout","The CGI script took too long to respond."),
 505:("HTTP Version Not Supported","The server does not support the HTTP version used in the request."),
}
T = """<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<title>{c} {r}</title>
<style>
body{{margin:0;min-height:100vh;display:flex;align-items:center;justify-content:center;
font-family:system-ui,-apple-system,"Segoe UI",sans-serif;background:#f4f4f1;color:#1d1d1b}}
main{{text-align:center;padding:2rem}}
h1{{font-size:6rem;margin:0;letter-spacing:-.04em;color:{col}}}
h2{{margin:.25rem 0 1rem;font-weight:600}}
p{{color:#5c5c58;max-width:28rem;margin:0 auto 1.5rem}}
a{{color:#1d1d1b;border:1px solid #1d1d1b;padding:.5rem 1rem;border-radius:4px;text-decoration:none}}
a:hover{{background:#1d1d1b;color:#f4f4f1}}
footer{{margin-top:2.5rem;font-size:.8rem;color:#9a9a95}}
</style>
</head>
<body>
<main>
<h1>{c}</h1>
<h2>{r}</h2>
<p>{d}</p>
<a href="/">Back to home</a>
<footer>webserv</footer>
</main>
</body>
</html>
"""
for c,(r,d) in codes.items():
    col = "#c0392b" if c>=500 else "#d35400"
    open(os.path.join(out,f"{c}.html"),"w").write(T.format(c=c,r=r,d=d,col=col))
