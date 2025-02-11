FROM ubuntu@sha256:1f1a2d56de1d604801a9671f301190704c25d604a416f59e03c04f5c6ffee0d6 AS custom
## Ubuntu 16.04

FROM pwn.red/jail

COPY --from=custom / /srv

COPY flag.txt /srv/app/
COPY chal /srv/app/run

EXPOSE 1337/tcp