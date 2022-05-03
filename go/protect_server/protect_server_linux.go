package protect_server

import (
	"fmt"
	"log"
	"net"
	"os"
	"os/signal"
	"reflect"
	"syscall"
)

func getOneFd(socket int) (int, error) {
	// recvmsg
	buf := make([]byte, syscall.CmsgSpace(4))
	_, _, _, _, err := syscall.Recvmsg(socket, nil, buf, 0)
	if err != nil {
		return 0, err
	}

	// parse control msgs
	var msgs []syscall.SocketControlMessage
	msgs, err = syscall.ParseSocketControlMessage(buf)

	if len(msgs) != 1 {
		return 0, fmt.Errorf("invaild msgs count: %d", len(msgs))
	}

	var fds []int
	fds, err = syscall.ParseUnixRights(&msgs[0])
	if len(fds) != 1 {
		return 0, fmt.Errorf("invaild fds count: %d", len(fds))
	}
	return fds[0], nil
}

// GetFdFromConn get net.Conn's file descriptor.
func GetFdFromConn(l net.Conn) int {
	v := reflect.ValueOf(l)
	netFD := reflect.Indirect(reflect.Indirect(v).FieldByName("fd"))
	pfd := reflect.Indirect(netFD.FieldByName("pfd"))
	fd := int(pfd.FieldByName("Sysfd").Int())
	return fd
}

func ServeProtect(path string, fwmark int) {
	os.Remove(path)
	defer os.Remove(path)

	l, err := net.ListenUnix("unix", &net.UnixAddr{Name: path, Net: "unix"})
	if err != nil {
		log.Fatal(err)
	}
	defer l.Close()

	os.Chmod(path, 0777)

	go func() {
		for {
			c, err := l.Accept()
			if err != nil {
				log.Println("Accept:", err)
				return
			}

			go func() {
				socket := GetFdFromConn(c)
				defer c.Close()

				fd, err := getOneFd(socket)
				if err != nil {
					log.Println("getOneFd:", err)
					return
				}

				if err := syscall.SetsockoptInt(int(fd), syscall.SOL_SOCKET, syscall.SO_MARK, fwmark); err != nil {
					log.Println("syscall.SetsockoptInt:", err)
				}

				if err == nil {
					c.Write([]byte{1})
				} else {
					c.Write([]byte{0})
				}
			}()
		}
	}()

	sigCh := make(chan os.Signal, 1)
	signal.Notify(sigCh, syscall.SIGINT, syscall.SIGTERM)
	<-sigCh
}
