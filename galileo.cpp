#include <iostream>
#include <cstdlib>

using namespace std;

int main() {

    cout << "Installing Apache" << endl;
    system("sudo pacman -S apache --noconfirm");
    system("sudo systemctl enable httpd");
    system("sudo systemctl start httpd");

    cout << "Installing PHP" << endl;
    system("sudo pacman -S php php-fpm php-gd --noconfirm");

    cout << "Installing MariaDB" << endl;
    system("sudo pacman -S mariadb --noconfirm");
    system("sudo mariadb-install-db --user=mysql --basedir=/usr --datadir=/var/lib/mysql");
    system("sudo systemctl enable mariadb");
    system("sudo systemctl start mariadb");

    cout << "Creating database and user" << endl;
    system("sudo mysql -u root -e \"CREATE DATABASE IF NOT EXISTS slims;\"");
    system("sudo mysql -u root -e \"CREATE USER IF NOT EXISTS 'slims_user'@'localhost' IDENTIFIED BY 'userslims';\"");
    system("sudo mysql -u root -e \"GRANT ALL PRIVILEGES ON slims.* TO 'slims_user'@'localhost';\"");
    system("sudo mysql -u root -e \"FLUSH PRIVILEGES;\"");

    cout << "Downloading and extracting SLiMS" << endl;
    system("wget https://github.com/slims/slims9_bulian/releases/download/v9.7.2/slims9_bulian-9.7.2.tar.gz");
    system("sudo tar -xf slims9_bulian-9.7.2.tar.gz -C /srv/http");
    system("sudo mv /srv/http/slims9_bulian-9.7.2 /srv/http/slims");
    system("sudo chown -R http:http /srv/http/slims");

    cout << "Configuring Apache" << endl;

    system("sudo cp /etc/httpd/conf/httpd.conf /etc/httpd/conf/httpd.conf.bak");

    system("sudo sed -i 's|^#LoadModule proxy_module modules/mod_proxy.so|LoadModule proxy_module modules/mod_proxy.so|' /etc/httpd/conf/httpd.conf");
    system("sudo sed -i 's|^#LoadModule proxy_fcgi_module modules/mod_proxy_fcgi.so|LoadModule proxy_fcgi_module modules/mod_proxy_fcgi.so|' /etc/httpd/conf/httpd.conf");
    system("sudo sed -i 's/^#LoadModule rewrite_module/LoadModule rewrite_module/' /etc/httpd/conf/httpd.conf");
    system("sudo sed -i 's/AllowOverride None/AllowOverride All/' /etc/httpd/conf/httpd.conf");
    system("sudo sed -i 's/DirectoryIndex index.html/DirectoryIndex index.php index.html index.htm/' /etc/httpd/conf/httpd.conf");

    system(R"(sudo tee -a /etc/httpd/conf/httpd.conf > /dev/null <<'EOF'

<FilesMatch "\.php$">
    SetHandler "proxy:unix:/run/php-fpm/php-fpm.sock|fcgi://localhost/"
</FilesMatch>

EOF)");

    cout << "Configuring PHP" << endl;

    system("sudo cp /etc/php/php.ini /etc/php/php.ini.bak");

    system("sudo sed -i 's/^;extension=mysqli/extension=mysqli/' /etc/php/php.ini");
    system("sudo sed -i 's/^;extension=gettext/extension=gettext/' /etc/php/php.ini");
    system("sudo sed -i 's/^;extension=pdo_mysql/extension=pdo_mysql/' /etc/php/php.ini");
    system("sudo sed -i 's/^;extension=gd/extension=gd/' /etc/php/php.ini");
    system("sudo sed -i 's/^;extension=xml/extension=xml/' /etc/php/php.ini");
    system("sudo sed -i 's/^;extension=mbstring/extension=mbstring/' /etc/php/php.ini");

    cout << "Starting PHP-FPM" << endl;
    system("sudo systemctl enable php-fpm");
    system("sudo systemctl start php-fpm");

    cout << "Restarting Apache" << endl;
    system("sudo systemctl restart httpd");

    cout << "SLiMS successfully installed!" << endl;

    return 0;
}
