# print("Content-type:text/html\r\n\r\n")
body = input()
print("<html>")
print("<head>")
print("<title>Hello World - First CGI Program</title>")
print("</head>")
print("<body>")
print("<h2>Hello World! Python</h2>")
if body != "":
    print("<h1>" + body + "<h1>")
print("</body>")
print("</html>")
