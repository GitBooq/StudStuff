# Services Network Data Exchange

Sender take text file, pass it to ipFilter and send to Receiver over gRPC.  
Receiver listen to gRPC port(50051) and output messages to console.  

# Dependencies

gRPC (if you build using cmake)

# Build&Run using 
## cmake

1. install gRPC
2. Run cmake
``` bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=<path_to_grpc>
cmake --build build
```
3. Run **receiver**
``` bash
cd build/src/receiver && \
./receiver
```
4. Run **sender** in another terminal  
You can set env variable `SERVER_HOST` to change server ip (default is `localhost`).  
``` bash
cd build/src/sender && \
./sender test.log
```

## docker

Build:
``` bash
docker compose build
```

Run **receiver** and show logs:
``` bash
docker compose up -d receiver && \
docker compose logs -f receiver
```

Run **sender** in another terminal:
``` bash
docker compose run sender test.log
```

### Output


*Receiver output:*  
```
Receiver server listening on 0.0.0.0:50051

=== Received Data ===
Source: ipv4_filter
Timestamp: 2026-06-07T11:08:05Z
Payload: 192.168.1.25 - GET /index.html
10.0.0.100 - POST /login
10.0.0.2 - GET /resource1.html

=====================

=== Received Data ===
Source: ipv4_filter
Timestamp: 2026-06-07T11:08:05Z
Payload: 10.0.0.3 - GET /resource2.html
10.0.0.4 - GET /resource3.html
10.0.0.5 - GET /resource4.html

=====================

=== Received Data ===
Source: ipv4_filter
Timestamp: 2026-06-07T11:08:05Z
Payload: 10.0.0.6 - GET /resource5.html
10.0.0.7 - GET /resource6.html
10.0.0.8 - GET /resource7.html

=====================

=== Received Data ===
Source: ipv4_filter
Timestamp: 2026-06-07T11:08:05Z
Payload: 10.0.0.9 - GET /resource8.html
192.168.1.0 - NETWORK_ADDR
192.168.1.255 - BROADCAST_ADDR

=====================

=== Received Data ===
Source: ipv4_filter
Timestamp: 2026-06-07T11:08:05Z
Payload: 10.0.0.1 - LOWER_BOUND
10.0.0.100 - UPPER_BOUND

=====================
```
Sender output is empty if no errors.