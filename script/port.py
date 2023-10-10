from time import sleep
import serial
import serial.tools.list_ports

def search_for_ports():
    ports = list(serial.tools.list_ports.comports())
    return ports

if __name__ == "__main__":
    print('available ports')
    for index, port in enumerate(search_for_ports()):
        print('[{}] {}'.format(index, port.description))

    print('\nselect port to connect (use index number)')
    
    while True:
        try:
            ser_device = input('> ')
            port = search_for_ports()[int(ser_device)].device
            break
        except:
            print('no valide port')

    while True:
        try:
            print('\nselect baudrate')
            baudrate = int(input('> '))
            break
        except:
            print('\nno valide baudrate')
    try:
        serial_conn = serial.Serial(port, baudrate)
    except Exception as e:
        print(e)
        print('\nCant connect to port {}'.format(port))
        exit(0)

    count = 0
    while not serial_conn.is_open:
        sleep(0.1)
        if count == 10:
            print('\nTimed out')
            exit(0)

    print('\nconnection established') 

    while serial_conn.is_open:
        try:
            print(serial_conn.readline())
        except:
            break

    print('\nconnection lost')
    exit(0)