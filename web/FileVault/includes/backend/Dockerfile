FROM php:7.4-apache

COPY src/. /var/www/html/

RUN sed -i 's/80/8080/g' /etc/apache2/sites-available/000-default.conf /etc/apache2/ports.conf

RUN apt-get update && apt-get install -y dnsutils zip unzip git

RUN docker-php-ext-install pdo_mysql

COPY --from=composer:latest /usr/bin/composer /usr/bin/composer

RUN composer install --no-dev --optimize-autoloader

RUN echo "upload_max_filesize = 10M" > /usr/local/etc/php/conf.d/uploads.ini && \
echo "post_max_size = 5M" >> /usr/local/etc/php/conf.d/uploads.ini && \
echo "max_execution_time = 20" >> /usr/local/etc/php/conf.d/security.ini && \
echo "memory_limit = 64M" >> /usr/local/etc/php/conf.d/security.ini


RUN echo "disable_functions = exec,system,shell_exec,passthru,proc_open,popen" > /usr/local/etc/php/conf.d/disable_functions.ini && \
    echo "default_socket_timeout = 30" >> /usr/local/etc/php/conf.d/custom.ini


CMD ["apache2-foreground"]
