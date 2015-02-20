# SimpleProxyServer

A simple HTTP proxysever written in C++.
Can only handle GET requests.
Does not cache data.
Does not support SSH.
Support blocked words, you can define any bad words that not are allowed but type them as argument at program start.
If user types in the blocked word in URL or gets it in content, user is redirected to an error page.

################################################################
##How to compile and run
There is a Makefile in the folder that are used to compile. Use following commands:

_Compile code:_

    $ make

_Run proxy server:_

    $ ./proxy <port no> <blocked word> <blocked word>

Alternatives:

    $ ./proxy <port no>		## No blocked words
    $ ./proxy 			## Will automatically set port number to 9000 with no blocked words

Example:

    $ ./proxy 4475 spongebob donaldduck britneyspears

Note that no uppercase words will work and spaces will make it into two words.

##How to configure Web Browser

 _Firefox:_
 Select Menu->Settings.
 Click on “Advanced” icon in Settings.
 Click on “Network” and then under “Connections” click on “Settings”.
 Select “Manual proxy configurations”
 At field “HTTP-proxy” enter the hostname and port where the proxy server is running. (eg hostname: 127.0.0.1 port: 9000)
 Make sure SSL, FTP and SOCKS protocol fields are empty and also the white box under.

##How to configure Firefox to use HTTP/1.0
 Proxy server can handle HTTP/1.1 so there is no need to do this. If you want to anyway, follow these steps:
 Type “about:config” in the title bar.
 Search for “network.http.proxy”.
 You should now see two keys, “...pipelining” and “...version”.
 Set pipelining to false. Set version to 1.0.
