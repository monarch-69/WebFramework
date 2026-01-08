FROM debian:bookworm-slim
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y --no-install-recommends build-essential make pkg-config ca-certificates iputils-ping \
    iproute2 && apt-get update && rm -rf /var/lib/apt/lists/*
    
WORKDIR /app
