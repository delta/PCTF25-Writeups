FROM ubuntu@sha256:0e5e4a57c2499249aafc3b40fcd541e9a456aab7296681a3994d631587203f97 AS custom
# Ubuntu 22.04

FROM ubuntu@sha256:0e5e4a57c2499249aafc3b40fcd541e9a456aab7296681a3994d631587203f97 AS builder 
# Ubuntu 22.04

RUN apt-get update && apt-get install -y python3 python3-pip python3-dev patchelf && rm -rf /var/lib/apt/lists/*

RUN pip3 install nuitka

COPY app.py /app.py

RUN nuitka --onefile --standalone --static-libpython=yes app.py



FROM pwn.red/jail

COPY --from=builder /app.bin /sock

COPY --from=custom / /srv

COPY flag.txt /srv/app/flag.txt
COPY chal /srv/app/chall

EXPOSE 1337/tcp

ENTRYPOINT [ "./sock" ]