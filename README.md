# Simple Search Engine
The main objective is to implement basic data-intensive application to index and search large documents. Data-intensive Computing and Cloud Computing are two emerging computing paradigms, which are poised to play an increasingly important role in the way Internet services are deployed and provided.
## Build & Run 
Run the master first, then any servers, then any clients.
### Master 
- Change directory into the “master” folder. Compile all the files with the pthread flag:  
`gcc -pthread -o master master.c`
- Run the newly created execution file :  
`./master`
- One new file will be generated, “master.txt”. Copy this file into the “server” and “client” folders.
### Server
- Copy the “server” folder into all corresponding machines.
- Change directory into the “server” folder and compile all the files:  
`gcc -o server server.c`
- Run the newly created execution file:   
`./server`
### Client
- Copy the “client” folder into all corresponding machines.
- Change directory into the “client” folder and compile all the files:  
`gcc -o client client.c`
