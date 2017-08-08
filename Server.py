# socket client
if __name__ == '__main__':
    import socket  # socket pack
    import time
    import os
    from OpenGL.GL import *
    from OpenGL.GLU import *
    from OpenGL.GLUT import *
    print "Socket Server Start"
    IMG_WIDTH = 160
    IMG_HEIGHT = 120
    cmd1 = ['\x01', '\xfe']
    cmd2 = ['\xfe', '\x01']
    img = [255 for i in range(IMG_WIDTH * IMG_HEIGHT)]

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  # start socket
    sock.bind(('192.168.43.5', 8080))
    sock.listen(5)  # start TCP listening
    connection, address = sock.accept()  # wait client connection
    print 'Connected by', address
    buf = connection.recv(2)

    def Draw():
        global img
        glClear(GL_COLOR_BUFFER_BIT)
        glPixelZoom(5, 5)
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1)
        img.reverse()
        glDrawPixels(IMG_WIDTH, IMG_HEIGHT, GL_LUMINANCE, GL_UNSIGNED_BYTE, img)
        glFlush()

    def IdleCallBack():
        global img
        buf = []
        tmp = [' ' for i in range(IMG_HEIGHT / 15)]
        while buf != cmd1[0]:
            buf = connection.recv(1)
        buf = connection.recv(1)
        if buf == cmd1[1]:
            connection.send('.')
            for i in range(IMG_HEIGHT / 15):
                tmp[i] = connection.recv(IMG_WIDTH * 15)
                connection.send(',')
            for j in range(len(tmp)):
                for k in range(len(tmp[j])):
                    img[k + j * len(tmp[j])] = ord(tmp[j][k])
            Draw()

    glutInit()
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB)
    glutInitWindowSize(IMG_WIDTH * 5, IMG_HEIGHT * 5)
    glutCreateWindow("ov2640 Camera")
    glutDisplayFunc(IdleCallBack)
    glutIdleFunc(IdleCallBack)
    glutMainLoop()

    connection.close()
