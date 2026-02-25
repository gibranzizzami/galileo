#include <iostream>
#include <cstdlib>
#include <string>

using namespace std;

int main() {
    string confirm;
    string dbName, dbUser, dbPass;

    cout << "=== SLiMS Auto Installer for Arch Linux ===" << endl;
    cout << "This program will install Apache, PHP, MariaDB, and SLiMS." << endl;
    cout << "Continue? (yes/no): ";
    cin >> confirm;

    if (confirm != "yes") {
        cout << "Installation cancelled." << endl;
        return 0;
    }

    // Input database configuration
    cout << "\nEnter database name [default: slims]: ";
    cin >> dbName;
    if (dbName.empty()) dbName = "slims";

    cout << "Enter database user [default: slims_user]: ";
    cin >> dbUser;
    if (dbUser.empty()) dbUser = "slims_user";

    cout << "Enter database password: ";
    cin >> dbPass;

    cout << "\nStarting installation...\n" << endl;

    cout << "Installing Apache..." << endl;
    system("sudo pacman -S apache --noconfirm");
    system("sudo systemctl enable httpd");
    system("sudo systemctl start httpd");

    cout << "Installing PHP..." << endl;
    system("sudo pacman -S php php-fpm php-gd --noconfirm");

    cout << "Installing MariaDB..." << endl;
    system("sudo pacman -S mariadb --noconfirm");
    system("sudo mariadb-install-db --user=mysql --basedir=/usr --datadir=/var/lib/mysql");
    system("sudo systemctl enable mariadb");
    system("sudo systemctl start mariadb");

    cout << "Creating database and user..." << endl;

    string createDB = "sudo mysql -u root -e \"CREATE DATABASE IF NOT EXISTS " + dbName + ";\"";
    string createUser = "sudo mysql -u root -e \"CREATE USER IF NOT EXISTS '" + dbUser + "'@'localhost' IDENTIFIED BY '" + dbPass + "';\"";
    string grantPriv = "sudo mysql -u root -e \"GRANT ALL PRIVILEGES ON " + dbName + ".* TO '" + dbUser + "'@'localhost';\"";

    system(createDB.c_str());
    system(createUser.c_str());
    system(grantPriv.c_str());
    system("sudo mysql -u root -e \"FLUSH PRIVILEGES;\"");

    cout << "\nDownload SLiMS now? (yes/no): ";
    cin >> confirm;

    if (confirm == "yes") {
        system("wget https://github.com/slims/slims9_bulian/releases/download/v9.7.2/slims9_bulian-9.7.2.tar.gz");
        system("sudo tar -xf slims9_bulian-9.7.2.tar.gz -C /srv/http");
        system("sudo mv /srv/http/slims9_bulian-9.7.2 /srv/http/slims");
        system("sudo chown -R http:http /srv/http/slims");
    } else {
        cout << "Skipping SLiMS download." << endl;
    }

    cout << "Configuring Apache..." << endl;

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

    cout << "Configuring PHP..." << endl;

    system("sudo cp /etc/php/php.ini /etc/php/php.ini.bak");
    system("sudo sed -i 's/^;extension=mysqli/extension=mysqli/' /etc/php/php.ini");
    system("sudo sed -i 's/^;extension=gettext/extension=gettext/' /etc/php/php.ini");
    system("sudo sed -i 's/^;extension=pdo_mysql/extension=pdo_mysql/' /etc/php/php.ini");
    system("sudo sed -i 's/^;extension=gd/extension=gd/' /etc/php/php.ini");
    system("sudo sed -i 's/^;extension=xml/extension=xml/' /etc/php/php.ini");
    system("sudo sed -i 's/^;extension=mbstring/extension=mbstring/' /etc/php/php.ini");

    cout << "Starting PHP-FPM..." << endl;
    system("sudo systemctl enable php-fpm");
    system("sudo systemctl start php-fpm");

    cout << "Restarting Apache..." << endl;
    system("sudo systemctl restart httpd");

    cout << "\n====================================" << endl;
    cout << "SLiMS Installation Completed!" << endl;
    cout << "Access via: http://localhost/slims" << endl;
    cout << "Database Name : " << dbName << endl;
    cout << "Database User : " << dbUser << endl;
    cout << "====================================" << endl;

    return 0;
}
