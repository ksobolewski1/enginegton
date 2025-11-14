
import chess
import subprocess
from collections import Counter # since when comparing move lists, there can be duplicates 


def get_board_state(board):
    checks = [
        (1, board.is_check),
        (2, board.is_checkmate),
        (3, board.is_stalemate),
        (4, board.is_insufficient_material),
        (5, board.is_seventyfive_moves),
        (6, board.is_fivefold_repetition),
        (7, board.is_game_over),
    ]

    for code, fn in checks:
        if fn():
            return code

    return 0


def move_gen_accuracy():

    test_set = []
    with open("./test/mga.txt", "r") as input_file:
        test_set = [line for line in input_file]

    fails = False
    for index, fen in enumerate(test_set):

        result = subprocess.run(
            ["./build/test", "0", fen],
            capture_output=True,
            text=True
        )

        # check the return code
        board = chess.Board(fen)
        python_moves = [move.uci() for move in board.legal_moves]
        engine_moves = result.stdout.split(";")
        if engine_moves[-1] == "":
            engine_moves.pop()

        state = get_board_state(board)
        eng_pos_state = int(engine_moves.pop())
        if state != eng_pos_state:
            print(f"inaccurate board state read: should be {state}, got {eng_pos_state}")
            cont = input("Continue? (y/n): ").strip().lower()
            if cont != 'y':
                print("test aborted")
                exit(0)

        pmc = Counter(python_moves)
        emc = Counter(engine_moves)
        if pmc != emc:
            fails = True
            print(f"\ninaccurate engine output at position {index + 1}\n")
            print(board)
            ep_diff = emc - pmc
            pe_diff = pmc - emc
            print(f"engine to python-chess difference: {list(ep_diff.elements())}")
            print(f"python-chess to engine difference: {list(pe_diff.elements())}\n")
            cont = input("Continue? (y/n): ").strip().lower()
            if cont != 'y':
                print("test aborted")
                exit(0)

    print("Success.") if not fails else print("Failure.")

available = {
        
        "mga": move_gen_accuracy

}
