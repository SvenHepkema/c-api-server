FROM ubuntu
WORKDIR /app
COPY target/implementation .
CMD ["./implementation"]
EXPOSE 8080
