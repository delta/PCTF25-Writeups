
# Stage 1: Build the Python application
FROM python:slim AS app
WORKDIR /app
COPY requirements.txt requirements.txt
RUN pip install --no-cache-dir -r requirements.txt

# Stage 2: Use pwn.red/jail to restrict the environment
FROM pwn.red/jail
COPY --from=app / /srv
COPY .env /srv/app/.env
COPY server_obf.py /srv/app/run
ENV JAIL_MEM=200M JAIL_TIME=60 JAIL_CPU=100
