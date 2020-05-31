## Version 0.5.0
* Allow viewing all members in a groupchat more conveniently in the management of the chat.
* Allow changing your nickname via GUI in your profile view.
* It is possible to search for other users via name, email or phone number if they opt into that feature (visibility setting).
* You configuration of the application and profile will be saved locally as JSON file ( "~/.config/cadet-gtk/config.json" ).
* Chats will use elliptic views for user names to allow the window size for mobile at all times.
* Changing your listening port was moved to advanced settings.
* The cat mode was added as practical feature to prevent others connecting to you without permission.
* Code is a lot cleaner again and some structs and methods were restructured for easier use and more consistency.

## Version 0.4.0
* Allow creating a groupchat inside the application with a selected port.
* Groupchats can listen on separate ports.
* Scrolling in chats is possible and text-input allows multiple lines.
* Code is cleaner than before and some warnings and errors were fixed.

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