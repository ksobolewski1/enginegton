import sys 
import chess # https://python-chess.readthedocs.io

def generate_out_file(filename, destination):

    # throw if either is not a valid path 
    
    positions = []
    with open(filename, "r") as in_file:
        positions = [line for line in in_file]

    all_moves = ""
    for pos in positions:
        # promotion is treated as a single move
        all_moves += ''.join({move.uci()[:4] for move in list(chess.Board(pos).legal_moves)}) + ';'

    with open(destination, "w") as out_file:
        out_file.write(all_moves)


if __name__ == "__main__":

    #throw if filename and/or dist not given 
    
    generate_moves(sys.argv[1], sys.argv[2])

