
import chess
import subprocess


def move_gen_correctness():
    
    # read in the file

    fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
    board = chess.Board(fen)
    moves = list(board.legal_moves)

    result = subprocess.run(
        ["./build/test", "0", fen],
        capture_output=True,
        text=True
    )

    print("STDOUT:", result.stdout)
    print("STDERR:", result.stderr)
    print("CODE:", result.returncode)

    # parse the engine's response and compare

available = {
        
        "mgc": move_gen_correctness

}
