FROM maven:3.8.4-openjdk-8 AS builder
WORKDIR /app
COPY . .
RUN mvn clean package -DskipTests

# Use Java 8 runtime for the final stage
FROM gcr.io/distroless/java:8
WORKDIR /app
COPY --from=builder /bin/sh /bin/sh
COPY --from=builder /bin/ls /bin/ls
COPY --from=builder /bin/cat /bin/cat
COPY --from=ghcr.io/tarampampam/curl:8.6.0 /bin/curl /bin/curl
COPY --from=builder /app/target/hr_app-0.0.1-SNAPSHOT.jar app.jar
COPY flag.txt /etc/flag.txt
EXPOSE 8080
ENTRYPOINT ["java", "-jar", "app.jar"]