FROM golang:1.24-alpine AS builder
WORKDIR /app
COPY go.mod ./
COPY *.go *.json ./
RUN go mod tidy && CGO_ENABLED=0 go build -ldflags "-w -s" -o /front44 .

FROM alpine:3.20
RUN apk --no-cache add tzdata ca-certificates wget
COPY --from=builder /front44 /front44
COPY static /static
ENV F44_STATIC_DIR=/static
EXPOSE 8080
ENTRYPOINT ["/front44"]
