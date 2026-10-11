# Builds and runs the C notice board inside a container.
# The gcc image already contains the compiler and make.
FROM gcc:13

WORKDIR /app
COPY . .

# Compile the C program (same as running "make" on your laptop).
RUN make

# Cloud platforms set the PORT environment variable themselves;
# the program reads it with getenv("PORT").
EXPOSE 8080
CMD ["./noticeboard"]
