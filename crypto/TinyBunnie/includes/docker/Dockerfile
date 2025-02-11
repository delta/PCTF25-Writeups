
# Stage 1: Build the Python application
FROM python:slim AS app
WORKDIR /app
COPY requirements.txt requirements.txt
RUN pip install --no-cache-dir -r requirements.txt



# Stage 2: Use pwn.red/jail to restrict the environment
FROM pwn.red/jail
COPY --from=app / /srv
COPY .env /srv/app/.env
COPY custom_block.py /srv/app/run
COPY tiny.py /srv/app/tiny.py


RUN chmod +x /srv/app/run

# ENV PATH="/srv/usr/local/bin:${PATH}"
ENV JAIL_MEM=200M JAIL_TIME=60 JAIL_CPU=100
# ENV OPENBLAS_NUM_THREADS=1

