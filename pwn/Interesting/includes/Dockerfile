FROM ubuntu@sha256:0e5e4a57c2499249aafc3b40fcd541e9a456aab7296681a3994d631587203f97 AS custom
## Ubuntu 22.04

FROM pwn.red/jail

COPY --from=custom / /srv

COPY bin/flag.txt /srv/app/
COPY bin/chal /srv/app/run

EXPOSE 1337/tcp