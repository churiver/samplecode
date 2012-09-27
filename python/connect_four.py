from random import *

# Red cell, Yellow cell, empty cell
R, Y, empty = 'R', 'Y', 'E'
# Draw case, add a chess, remove a chess
draw, add, remove = 'D', 'A', 'R'
# Undo the last pair moves, quit the game, reset the game
undo, quit, reset = 'U', 'Q', 'N'
# Chess board size
rowno, colno = 6, 7


def printboard(board):
    for y in range(rowno-1, -1, -1):
        for x in range(colno):
            try:
                value = board[x][y]
            except IndexError:
                value = ' '
            print('|{0}'.format(value), end='')
        print("|")
    print('-'*(2*colno+1))
    [print(' {0}'.format(i), end='') for i in range(colno)]
    print()


# Check if the move is valid
def isvalid(board, diskcolor, move):
    if len(move) == 2 and '0' <= move[1] <= str(colno):
        col = int(move[1])
        if move[0] == add:
            if(len(board[col]) < rowno):
                return True
        elif move[0] == remove:
            if(len(board[col]) > 0 and board[col][0] == diskcolor):
                return True
    return False


def move(board, diskcolor, move):
    if(isvalid(board, diskcolor, move)):
        col = int(move[1])
        if move[0] == add:
            board[col].append(diskcolor)
        else:
            board[col].pop(0)
    else:
        raise ValueError("Invalid move")


def undomove(board, diskcolor, move):
    col = int(move[1])
    if move[0] == add:
        board[col].pop()
    else:
        board[col].insert(0, diskcolor)


# line: ['Y', 'Y', 'R'] etc.
def getcolor4(line):
    counts = {R:0, Y:0, empty:0}
    maxcounts = {R:0, Y:0, empty:0}
    lastcolor = R

    for color in line:
        if color != lastcolor:
            maxcounts[lastcolor] = counts[lastcolor] if maxcounts[lastcolor] \
                < counts[lastcolor] else maxcounts[lastcolor]
            counts[lastcolor] = 0
            lastcolor = color
        counts[color] += 1
    for color in maxcounts.keys():
        if maxcounts[color] < counts[color]:
            maxcounts[color] = counts[color]

    # Remove empty counts from maxcounts    
    maxcounts.pop(empty)
    # Return Y/R/Y&R. The last case is a draw
    return [color for color in maxcounts.keys() if maxcounts[color] >3]


# Calculate the possible win case in 4 directions
def getwinner(board):
    board2d = [list(b) for b in board]
    [col.extend([empty]*(colno-len(col))) for col in board2d if len(col)<colno]
    winner = set()

    # Horizontal line
    for x in range(colno):
        colors = getcolor4(board2d[x])
        if len(colors) != 0:
            [winner.add(color) for color in colors]

    # Vertical line
    for y in range(rowno):
        vline = [col[y] for col in board2d]
        colors = getcolor4(vline)
        if len(colors) != 0:
            [winner.add(color) for color in colors]

    # Diagonal line
    for i in range(3, colno+rowno-4):
        dline = [board2d[i-y][y] for y in range(i) if 0<=i-y<colno and y<rowno]
        colors = getcolor4(dline)
        if len(colors) != 0:
            [winner.add(color) for color in colors]

    # Reverse diagonal line
    for i in range(4-rowno, colno-3):
        rdline = [board2d[x][x-i] for x in range(colno) if 0<=x-i<rowno]
        colors = getcolor4(rdline)
        if len(colors) != 0:
            [winner.add(color) for color in colors]

    return winner


def getusermove(board, diskcolor):
    while True:
        cmd = input('Input Your command:')
        if isvalid(board, diskcolor, cmd):
            return cmd
        elif cmd[0] == undo or quit or reset:
            return cmd[0]


# Here computer computes next move with random
def getcomputermove(board, diskcolor):
    while True:
        movecol = randint(0, colno-1)
        # 4/5 possibilitiy to add
        movetype = add if randint(1, 5) > 1 else remove
        cmd = movetype + str(movecol)
        if isvalid(board, diskcolor, cmd):
            return cmd
        elif cmd[0] == undo or quit or reset:
            return cmd[0]


def play(redmovefn, yellowmovefn):
    winner, moves, board = [], [], [[] for i in range(colno)]
    players = {R:('Red', redmovefn), Y:('Yellow', yellowmovefn)}
    colors = [R, Y]
    print('--------------------------------------')
    print('-----Welcome to play Connect Four-----')
    print('------Copyright: Li Yu. Ver: 0.5------')

    while True:
        stop = False
        for c in colors:
            color, name, fn = c, players[c][0], players[c][1]
            print('\n[System:] Round {0}'.format(len(moves) + 1))
            printboard(board)
            print('[System:] {0} player\' turn: '.format(name))
            cmd = fn(board, color)
            if cmd == undo:
                undomove(board, moves[-1][:1], moves[-1][1:])
                moves.pop()
                undomove(board, moves[-1][:1], moves[-1][1:])
                moves.pop()
                print('[System:] {0} undo the last pair of moves'.format(name))
            elif cmd == quit:
                print('[System:] {0} quit the game.'.format(name))
                stop = True
                break
            elif cmd == reset:
                print('[System:] {0} reset the game'.format(name))
                board, moves = [], []
            elif isvalid(board, color, cmd):
                print('[System:] {0} made a move'.format(name))
                move(board, color, cmd)
                moves.append(color + cmd)
                winner = getwinner(board)
                # print('[System:] Winner {0}, len {1}'.format((winner), len(winner)))
                if len(winner) != 0:
                    stop = True
                    break
        if stop:
            break

    print('[System:] Game over.')
    moves = ['[System:] {0} made a move of {1}'.format(players[m[0]][0], m[1:]) for m in moves]
    winner = [players[w][0] for w in winner]
    return winner, moves, board


if __name__ == "__main__":
    winner, moves, board = play(getusermove, getcomputermove)
    print("Winner: ", winner)
    print("Moves: ", moves)
    print("Board: ", board)