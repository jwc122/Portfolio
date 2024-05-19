import Game_module from "../Game_module.js";
import R from "../ramda.js";

// All entries are powers of 2: (0), 2, 4, 8, 16, etc.
const is_valid_token = function(token) {
    if (!Number.isInteger(token)) {
        return false;
    }
    if (token === 0) {
        return true;
    } else {
        return Math.pow(2, Math.round(Math.log(token) / Math.log(token)));
    }
};

function countTotalCells(board) {
    let foundNonZero = false;
    board.forEach(function(row) {
        row.forEach(function(cell) {
            if (cell !== 0) {
                foundNonZero = true;
            }
        });
    });
    return foundNonZero;
}


function isGameEnded(uboard) {
    // Check if the board is full
    if (!Game_module.hasEmptyTile(uboard)) {
        // Check if no adjacent identical tiles exist
        const rows = uboard.length;
        const columns = uboard[0].length;
        let gameEnded = true;
        uboard.forEach(function(row, r) {
            row.forEach(function(_, c) {
                const currentTile = uboard[r][c];
                // Check for adjacent identical tiles above, below, left, right
                if (
                    // above
                    (r > 0 && uboard[r - 1][c] === currentTile) ||
                    //below
                    (r < rows - 1 && uboard[r + 1][c] === currentTile) ||
                    // left
                    (c > 0 && uboard[r][c - 1] === currentTile) ||
                    //right
                    (c < columns - 1 && uboard[r][c + 1] === currentTile)
                ) {
                    gameEnded = false; // Game is not ended
                }
            });
        });
        return gameEnded; // Game is ended if no adjacent identical tiles found
    }
}

// Utility function to compare two arrays
function arraysEqual(a, b) {
    return JSON.stringify(a) === JSON.stringify(b);
  }
describe("board", function () {
    it("The board is not a 2D array", function () {
        const board = Game_module.init();
        const throw_if_invalid = function(board) {
            // 2-D array
        };
        if (!Array.isArray(board) || !Array.isArray(board[0])) {
            throw new Error(
                "The board is not a 2D array"
            );
        }
    });
    it("The board length is not 4", function () {
        const width = Game_module.init().length;
        if (!(width === 4)) {
            throw new Error(
            "The board length is not 4"
            );
        }
    });
    it("The board is not square", function () {
        // Square - 4x4
        const board = Game_module.init();
        const square = R.all(
            (column) => column.length === 4,
            board
        );
        if (!square) {
            throw new Error(
                "The board is not square"
            );
        }
    });
    it("Board contains invalid tokens", function () {
        const board = Game_module.init();
        const contains_valid_tokens = R.pipe(
            R.flatten,
            R.all((slot) => is_valid_token(slot))
            )(board);
        if (!contains_valid_tokens) {
            throw new Error(
                "Board contains invalid tokens"
            );
        }
    });
    it("should return an empty board", function() {
        const board = Game_module.init();
        const totalCells = countTotalCells(board);
        if (!(totalCells == false)) {
          throw new Error(
            `Expected board to be empty`);
        }
    });
});
describe("full board", function () {
    /* if the board is empty dont end the game - testing if game over*/
    it("check if board is full and game is over", function () {
        const empty_board = [
            [4, 2, 4, 8],
            [2, 64, 8, 4],
            [8, 16, 32, 64],
            [4, 2, 4, 8]
        ];
        if (!isGameEnded(empty_board)) {
          throw new Error("The game should be over");
        }
    });
    it("2 tiles are same value adjacently, can move left", function () {
        const move_left = [
          [4, 2, 4, 8],
          [2, 8, 8, 4],
          [8, 16, 32, 64],
          [4, 2, 4, 8]
        ];
        if (isGameEnded(move_left)) {
          throw new Error("The game is over, you cannot move left");
        }
    });
    it("2 tiles are same value adjacently, can still move right", function () {
        const move_right = [
          [4, 64, 4, 8],
          [2, 2, 16, 4],
          [8, 16, 32, 64],
          [4, 2, 4, 8]
        ];
        if (isGameEnded(move_right)) {
          throw new Error("The game is over, you cannot move right");
        }
    });
    it("2 tiles same value stacked, can still move up", function () {
        const move_up = [
          [4, 2, 4, 8],
          [2, 8, 64, 4],
          [4, 16, 32, 64],
          [4, 2, 4, 8]
        ];
        if (isGameEnded(move_up)) {
          throw new Error("The game is over, you cannot move up");
        }
    });
    it("2 tiles same value stacked, can still move down", function () {
        const move_down = [
          [4, 2, 4, 8],
          [2, 8, 64, 4],
          [4, 16, 32, 64],
          [4, 2, 4, 8]
        ];
        if (isGameEnded(move_down)) {
          throw new Error("The game is over, you cannot move down");
        }
    });
});
describe("check if move valid", function () {
    it("moving left is valid", function () {
    const board = [
        [0, 0, 2, 2],
        [0, 0, 0, 2],
        [0, 0, 4, 4],
        [0, 0, 0, 0]
    ];

    const expectedBoard = [
        [4, 0, 0, 0],
        [2, 0, 0, 0],
        [8, 0, 0, 0],
        [0, 0, 0, 0]
    ];

    const result = Game_module.slideLeft(board);

    // Check if resulting board after sliding left is equal to expected board
    if (!arraysEqual(result.uboard, expectedBoard)) {
        throw new Error("Moving left is not valid");
    }
    });
    it("moving right is valid", function () {
        const board = [
            [0, 0, 2, 2],
            [0, 0, 0, 2],
            [0, 0, 4, 4],
            [0, 0, 0, 0]
        ];
        const expectedBoard = [
            [0, 0, 0, 4],
            [0, 0, 0, 2],
            [0, 0, 0, 8],
            [0, 0, 0, 0]
        ];
        const result = Game_module.slideRight(board);
        //Check if resulting board after sliding left is equal to expected board
        if (!arraysEqual(result.uboard, expectedBoard)) {
            throw new Error("Moving right is not valid");
        }
        });
            it("moving up is valid", function () {
        const board = [
            [0, 0, 2, 2],
            [0, 0, 0, 2],
            [0, 0, 4, 4],
            [0, 0, 0, 0]
        ];
        const expectedBoard = [
            [0, 0, 2, 4],
            [0, 0, 4, 4],
            [0, 0, 0, 0],
            [0, 0, 0, 0]
        ];
        const result = Game_module.slideUp(board);
        //Check if resulting board after sliding left is equal to expected board
        if (!arraysEqual(result.uboard, expectedBoard)) {
            throw new Error("Moving up is not valid");
        }
        });
    it("moving down is valid", function () {
        const board = [
            [0, 0, 2, 2],
            [0, 0, 0, 2],
            [0, 0, 4, 4],
            [0, 0, 0, 0]
        ];
        const expectedBoard = [
            [0, 0, 0, 0],
            [0, 0, 0, 0],
            [0, 0, 2, 4],
            [0, 0, 4, 4]
        ];
        const result = Game_module.slideDown(board);
        //Check if resulting board after sliding left is equal to expected board
        if (!arraysEqual(result.uboard, expectedBoard)) {
            throw new Error("Moving down is not valid");
        }
    });
});

describe("empty board", function () {
    it("score is 0 on an empty board", function () {
    const emptyBoard = [
        [0, 0, 0, 0],
        [0, 0, 0, 0],
        [0, 0, 0, 0],
        [0, 0, 0, 0]
    ];
    const expectedResult = {
        uboard: [
        [0, 0, 0, 0],
        [0, 0, 0, 0],
        [0, 0, 0, 0],
        [0, 0, 0, 0]
        ],
        score: 0
    };
    const result = Game_module.slideLeft(emptyBoard);
    // Check if resulting board after moving left is equal to the expected board
    if (!arraysEqual(result.uboard, expectedResult.uboard)) {
        throw new Error("Moving left on an empty board is not valid");
    }
    // Check if the score is 0
    if (result.score !== expectedResult.score) {
        throw new Error("Score on empty board not 0");
    }
    });
});

// Couldn't get test to work
//function testSetTwo(uboard) {
//     // Call the setTwo function to generate a new number
//     let updatedBoard = Game_module.setTwo(uboard);
//     // Check if any element in the updated board differs from original board
//     let differingElementFound = false;
//     updatedBoard.forEach((row, r) => {
//       row.forEach((cell, c) => {
//         if (cell !== uboard[r][c]) {
//           differingElementFound = true;
//         }
//       });
//     });
//     return !differingElementFound; // Returns true if no new number generated
// }
// // Throw an error if no new number is generated
// it("random", function () {
//     let board = [
//         [2, 4, 0, 0],
//         [0, 0, 8, 2],
//         [0, 0, 0, 4],
//         [0, 0, 0, 4]
//       ];
//     if (!testSetTwo(board)) {
//       throw new Error("No new number (2 or 4) generated!");
//     }
// });