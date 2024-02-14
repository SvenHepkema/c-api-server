# c-api-server

This repository contains a library that allows the user to setup a server using C. The user will be able to create an JSON REST API server 
by registering paths and associating them with callbacks to handle incoming requests.
As this is a hobby project, it is not meant for production use. The focus at the end will be on performance, 
developing different approaches to get the most performance. This will be benchmarked and documented.

This project expands upon the c server that I built for my [tiny-django](https://github.com/SvenHepkema/tiny-django) project. It also uses [my own c implementation](https://github.com/SvenHepkema/c-threadpool) of a thread pool.

# Benchmarking

Benchmarking is done using [Apache Benchmark](https://httpd.apache.org/docs/2.4/programs/ab.html) to measure if performance improvements actually improve performance. All tests are done using the scripts in the benchmark-scripts repository.
 
# Functionality

The envisioned functionality will be:

- Associating paths with callbacks.
- Supports json responses.
- Optimization for latency and load handling, with benchmarking scripts to test improvements.
- A user-friendly API.
