
import chess
import subprocess
from collections import Counter # since when comparing move lists, there can be duplicates 


def move_gen_accuracy():
    
    # read in the file

    fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

    result = subprocess.run(
        ["./build/test", "0", fen],
        capture_output=True,
        text=True
    )

    # check if return code was fine

    python_moves = [move.uci() for move in chess.Board(fen).legal_moves]
    engine_moves = result.stdout.split(";")

    if Counter(python_moves) != Counter(engine_moves):
        print(result.stdout)
        print("wrong engine output")
        # ask if test should continue

available = {
        
        "mga": move_gen_accuracy

}
