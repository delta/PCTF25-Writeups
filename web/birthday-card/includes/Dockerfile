FROM debian:buster-slim  AS build
RUN apt-get update && \
    apt-get install --no-install-suggests --no-install-recommends --yes build-essential libpq-dev python3-venv gcc libpython3-dev coreutils netcat && \
    python3 -m venv /venv && \
    /venv/bin/pip install --upgrade pip setuptools wheel

RUN adduser ICanDoThisAllDay --force-badname

FROM build AS build-venv
COPY requirements.txt /requirements.txt
RUN /venv/bin/pip install --disable-pip-version-check -r /requirements.txt

FROM busybox:1.35.0-uclibc as busybox

FROM gcr.io/distroless/python3-debian10
COPY --from=build-venv /venv /venv
COPY --from=build /etc/passwd /etc/passwd
COPY . /app

WORKDIR /app

USER ICanDoThisAllDay

ENTRYPOINT ["/venv/bin/gunicorn", "-b", "0.0.0.0:5000", "app.app:app", "-w", "2", "--worker-class", "gevent", "--timeout", "60"]