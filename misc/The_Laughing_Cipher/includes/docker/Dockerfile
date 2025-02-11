FROM golang:alpine AS builder
RUN apk add --no-cache git
WORKDIR /app
RUN go mod init decrypt-oracle
RUN go get github.com/gin-gonic/gin
COPY . .
RUN go mod tidy
RUN CGO_ENABLED=0 GOOS=linux go build -o main .

FROM alpine:latest
RUN apk --no-cache add ca-certificates
WORKDIR /root/
COPY --from=builder /app/main .
CMD ["./main"]