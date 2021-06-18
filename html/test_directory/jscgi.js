
qs = process.env.QUERY_STRING;
let vars = qs.split('&');
let name = "";
for (var i = 0; i < vars.length; i++) {
    var pair = vars[i].split('=');
    if (pair[0] == "name")
        name = pair[1];
}
console.log("<html>");
console.log("<head>");
console.log("<title>Hello World</title>");
console.log("</head>");
console.log("<body>");
if (name != "")
    console.log("<h1>Hello, " + name + "!</h1>");
else
    console.log("<h1>Hello</h1>");
console.log("</body>");
console.log("</html>");