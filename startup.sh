#!/bin/bash

# Starting the RabbitMQ Server
CONF_ENV_FILE="/home/linuxbrew/.linuxbrew/etc/rabbitmq/rabbitmq-env.conf" /home/linuxbrew/.linuxbrew/opt/rabbitmq/sbin/rabbitmq-server &

# Starting Node JS Server
node server &

# # Wait for any process to exit
wait -n

# Exit with status of process that exited first
# exit $?