FROM --platform=linux/amd64 ubuntu:22.04

# Set environment variables to avoid interactive prompts
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Etc/UTC

# Install required dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    gcc \
    make \
    gdb \
    expect \
    grep \
    && rm -rf /var/lib/apt/lists/*

# Set up working directory
WORKDIR /app

# Copy the ARMv8 simulator project
COPY TP1-ARM/ ./

# Make scripts executable
RUN chmod +x ref_sim_x86

# Set the default command to open a shell
CMD ["/bin/bash"] 