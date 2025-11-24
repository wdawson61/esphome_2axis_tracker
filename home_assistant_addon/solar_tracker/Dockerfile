ARG BUILD_FROM
FROM $BUILD_FROM

ENV LANG C.UTF-8

# Install requirements
RUN apk add --no-cache \
    python3 \
    py3-pip \
    py3-numpy \
    py3-scipy \
    tzdata

# Copy requirements and install Python packages
COPY requirements.txt /tmp/
RUN pip3 install --no-cache-dir -r /tmp/requirements.txt

# Copy application files
WORKDIR /app
COPY run.sh /
COPY solar_tracker/ /app/solar_tracker/
COPY web/ /app/web/

RUN chmod a+x /run.sh

CMD [ "/run.sh" ]
