## Version 0.3.0
* Integrating a layer with JSON or similar to allow metadata in messages.
* Integrating the current state of functionality of the groupchat-application (client-side).
>  source is here: https://git.gnunet.org/groupchat.git/tree/
* You will use your username as nickname for sending JSON messages instead of "Me".

## Version 0.2.0
* Ports make channels and chats completely independent of each other.
* Fixed all warnings to secure the application.
* Fixed minor issue with stack changing chats initially.

## Version 0.1.0
* The first target of this application is being able to communicate with the basic cli for CADET.
>  source is here: https://git.gnunet.org/gnunet.git/tree/src/cadet/gnunet-cadet.c
* The second target is creating a channel towards another peer.
* An advanced target is to handle multiple channels at once.
* Choosing a specific port instead of a default ('test') one.