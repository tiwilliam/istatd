About istatd

istatd is a daemon serving statistics to your iStat iPhone application from Linux, Solaris & FreeBSD. istatd collects data such as CPU, memory, network and disk usage and keeps the history. Once connecting from the iPhone and entering the lock code this data will be sent to the iPhone and shown in fancy graphs.

What is iStat for iPhone?

iStat is a iPhone application developed by Bjango <http://www.bjango.com/>. With iStat you can remotely monitor CPU, memory, disks, uptime and load averages from any Mac, Linux or Solaris computer from your iPhone. You can download iStat in iTunes App Store.

How to install

These steps assume you are logged in as root and have GNU build tools installed together with libxml2.

1) Download istatd-x.x.x.tar.gz

   # wget http://github.com/downloads/tiwilliam/istatd/istatd-x.x.x.tar.gz

2) Extract tar ball

   # tar -xvf istatd-x.x.x.tar.gz

3) Install dependencies

   istatd requires GNU build tools, libxml2 and libxml2-devel.
   Depending on your distribution these packages can be diffrently named.

   CentOS
   # yum install libxml2-devel

   Ubuntu
   # apt-get install libxml2-dev

4) Build istatd

   # cd istatd-x.x.x

   Here you can configure where you want your binary and config to end up (default /usr/local).
   # ./configure
   or
   # ./configure --prefix=/ --sysconfdir=/etc

   # make
   # make install

5) Add user and configure directories

   # useradd istat
   # mkdir -p /var/{run,cache}/istat
   # chown istat.istat /var/{run,cache}/istat

6) Configure your config to match your needs and system

   # vim /etc/istat.conf

   Note: Don't forget to change your server_code.

7) Fire it up

   # /usr/bin/istatd -d
