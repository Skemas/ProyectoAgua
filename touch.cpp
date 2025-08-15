#include "touch.h"

// Devuelve true si el toque está dentro del rectángulo
bool touchDentro(int bx, int by, int bw, int bh, int tx, int ty) {
    return (tx >= bx && tx <= bx + bw && ty >= by && ty <= by + bh);
}
