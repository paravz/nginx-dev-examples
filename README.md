## Modules

### hello_world

Installs location content handler to produce specific output.

- #1 produces the predefined output "Hello, world!"
- #2 allows setting text and HTTP status code
- #3 supports variables in output text

### access

Installs an ACCESS phase handler and checks if user is allowed to access the
resource.

- #1 makes sure the User-Agent header contains the specified string
- #2 verifies user-provided hash md5(uri, secret).

### set_header

Installs header filter handler and allows to add output headers.

- #1 one header is supported
- #2 multiple headers are supported
- #3 variables are supported in header values

### append

Installs header and body filter handlers and allows appending text to the
output.

- #1 a string is appended
- #2 md5 hash of the entire body is appended
- #3 subrequest text is appended

### md5

Creates a variable with the md5 of the provided complex value.

## Development Guide

For more information on module development, see the NGINX development guide:

    http://nginx.org/en/docs/dev/development_guide.html

## VScode Development Environment on Linux with gcc

- Clone nginx into "nginx" directory

  `git clone https://github.com/nginx/nginx` or  
  `hg clone http://hg.nginx.org/nginx`

- Install C/C++ IntelliSense

  Install [ms-vscode.cpptools](https://github.com/Microsoft/vscode-cpptools/releases) extension using GUI or `code --install-extension ms-vscode.cpptools`

- Open this directory in VScode and start editing any module's source file

- Configure and build nginx binary with the currently edited module compiled in

  Go to "Terminal > Run Task" and choose "initial: configure and build active module"

  Alternatively press "Ctrl + p", type "task" and select "initial: configure and build active module"

- Make an edit to module's source file and rebuild incrementally
  Run default build task: "Ctrl + Shift + B", or choose "Terminal > Run Build Task..."

  Build task stops nginx if it's running, use "Ctrl + Shift + B" to stop nginx.

- Run and debug

  Press "F5" to start nginx under gdb debugger.

  Set a breakpoint in module's handler and make a request: `curl localhost:8000`

  Press "Shift F5" to stop nginx.

- Sample debug session

  ![Debugging Example](/vscode1.png?raw=true "Breakpoint in access_1 module")