from ctypes import *
import mmap
from posix_ipc import SharedMemory, ExistentialError

# Load the shared library
lib = cdll.LoadLibrary('./hash_map.so')

# Define the VFT and VFD structures
class VFT(Structure):
    _fields_ = [
        ('tipo_dia', c_int),
        ('cod_variante', c_char_p),
        ('frecuencia', c_char_p),
        ('cod_ubic_parada', c_char_p),
        ('ordinal', c_char_p),
        ('hora', c_char_p),
        ('dia_anterior', c_char_p),
        ('X', c_char_p),
        ('Y', c_char_p)
    ]

class VFD(Structure):
    _fields_ = [
        ('id', c_char_p),
        ('codigoEmpresa', c_char_p),
        ('frecuencia', c_char_p),
        ('codigoBus', c_char_p),
        ('variante', c_char_p),
        ('linea', c_char_p),
        ('sublinea', c_char_p),
        ('tipoLinea', c_char_p),
        ('destino', c_char_p),
        ('subsistema', c_char_p),
        ('version', c_char_p),
        ('velocidad', c_char_p),
        ('latitud', c_char_p),
        ('longitud', c_char_p),
        ('fecha', c_char_p)
    ]

class Entry(Structure):
    pass

Entry._fields_ = [
    ('key', c_char_p),
    ('vfts', POINTER(POINTER(VFT))),
    ('vfds', POINTER(POINTER(VFD))),
    ('vft_count', c_size_t),
    ('vft_capacity', c_size_t),
    ('vfd_count', c_size_t),
    ('vfd_capacity', c_size_t),
    ('next', POINTER(Entry))
]

class HashMap(Structure):
    _fields_ = [
        ('buckets', POINTER(POINTER(Entry))),
        ('size', c_size_t),
        ('count', c_size_t),
        ('campos_capturas', POINTER(c_char_p)),
        ('campos_horarios', POINTER(c_char_p))
    ]

SHM_NAME = '/vfd_map_shm'
SHM_SIZE = 4096  # Adjust size according to your needs

def main():
    try:
        shm = SharedMemory(SHM_NAME)
        with mmap.mmap(shm.fd, shm.size) as mm:
            # Read the raw data
            raw_data = mm.read(shm.size)

            # Create a ctypes pointer to the shared memory
            hashmap_ptr = cast(raw_data, POINTER(HashMap))
            # Access the contents
            hashmap = hashmap_ptr.contents
            # Print or process the data
            print(f"HashMap size: {hashmap.size}")
            print(f"HashMap count: {hashmap.count}")

            # Print or process the data
            print(f"HashMap size: {hashmap.size}")
            print(f"HashMap count: {hashmap.count}")
    except ExistentialError:
        print(f"Shared memory object '{SHM_NAME}' not found.")
    except Exception as e:
        print(f"Error: {e}")

if __name__ == '__main__':
    main()
