package neko_log

import (
	"io"
	"log"
	"os"
)

var f_neko_log *os.File

var LogWriter *logWriter

func SetupLog(maxSize int, path string) {
	if f_neko_log != nil && LogWriter != nil {
		return
	}
	// Truncate mod from libcore, simplify because only 1 proccess.
	oldBytes, err := os.ReadFile(path)
	if err == nil && len(oldBytes) > maxSize {
		if os.Truncate(path, 0) == nil {
			oldBytes = oldBytes[len(oldBytes)-maxSize:]
		}
	}
	// open
	f_neko_log, err = os.OpenFile(path, os.O_RDWR|os.O_APPEND|os.O_CREATE, 0644)
	if err == nil {
		f_neko_log.Write(oldBytes)
	} else {
		log.Println("error open log", err)
	}
	//
	LogWriter = &logWriter{
		files: []io.Writer{os.Stdout, f_neko_log},
	}
	// setup std log
	log.SetFlags(log.LstdFlags | log.LUTC)
	log.SetOutput(LogWriter)

}

type logWriter struct {
	files []io.Writer
}

func (w *logWriter) Write(p []byte) (n int, err error) {
	for _, file := range w.files {
		if file == nil {
			continue
		}
		n, err = file.Write(p)
		if err != nil {
			return
		}
	}
	return
}
