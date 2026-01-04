FROM debain:bookworm-slim
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y --no-install-recommends build-essential make pkg-config && \
    rm -rf /var/lib/apt/lists/*
    
WORKDIR /app

EXPOSE 6969

CMD ["make", "./server"]
