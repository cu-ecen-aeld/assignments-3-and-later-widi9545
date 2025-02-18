#!/bin/sh
# https://unix.stackexchange.com/questions/20357/how-can-i-make-a-script-in-etc-init-d-start-at-boot


start() {
    start-stop-daemon -S -n aesdsocket --exec /usr/bin/aesdsocket -- -d

}
   
stop() {
    start-stop-daemon --stop --signal SIGTERM --name aesdsocket --retry 3 

}

restart(){
    stop
    sleep 10
    start
}
   
case "$1" in 
    start)
       start
       ;;
    stop)
       stop
       ;;
    restart)
        restart

       ;;
    *)
       echo "Usage: $0 {start|stop|restart}"
esac
   
exit 0 
