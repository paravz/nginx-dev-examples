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


## NGINX Development and Debugging in VS Code

Learning NGINX modules is a lot easier in [VS Code](https://code.visualstudio.com/), use instructions below to setup your Linux C/C++ development environment.

  ![NGINX VScode Debugging session](/vscode1.png?raw=true "Breakpoint in access_1 NGINX module")

### Prepare

- Download NGINX into *nginx* directory

  `git clone https://github.com/nginx/nginx`

  or

  `hg clone http://hg.nginx.org/nginx`

- Install dependencies

    - Install NGINX [build dependencies](https://docs.nginx.com/nginx/admin-guide/installing-nginx/installing-nginx-open-source/#installing-nginx-dependencies) or use your package manager: `sudo dnf install -y pcre-devel zlib-devel openssl-devel`

    - Install [C/C++ IntelliSense extension](https://github.com/Microsoft/vscode-cpptools/releases) in VS Code GUI or run from terminal: `code --install-extension ms-vscode.cpptools`

    - Install GDB: `sudo dnf install -y gdb`

- Open *access_1/ngx_http_ua_access_module.c* in VS Code

- Run initial NGINX build (one time)

    Navigate to "Terminal > Run Task" and choose `nginx: initial configure and build active module`

### Develop and Debug

- Open *access_1/ngx_http_ua_access_module.c* to compile *access_1* module

- Run default build task using: "Ctrl + Shift + B" or choose `Terminal > Run Build Task...`

    This task from *.vscode/tasks.json* builds NGINX with module that is currently active in editor and stops NGINX.

- Run and/or debug

    - Press "F5" to start nginx under GDB. GDB launch configration is defined in *.vscode/launch.json*

    - Set a breakpoint in module's handler (ie *access_1/ngx_http_ua_access_module.c:76*) and make a request: `curl localhost:8000`
        - See [VS Code debugging documentation](https://code.visualstudio.com/docs/cpp/cpp-debug) for more details

    - Press "Shift F5" to stop debugging session.
