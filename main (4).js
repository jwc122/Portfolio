import R from "./ramda.js";
import Game_module from "./Game_module.js";

var board;
var score = 0;

board = Game_module.init();

window.onload = function() {
    setGame();
};

function updateTile(tile, num) {
    // Clear the content of the tile element.
    tile.innerText = "";
    // Clear the classList of the tile element.
    tile.classList.value = "";
    // Add the "tile" class to the tile element.
    tile.classList.add("tile");
    // Check if num is greater than 0.
    if (num > 0) {
        // Set the content of the tile element to num as a string.
        tile.innerText = num.toString();
        // Check if num is less than or equal to 4096.
        if (num <= 4096) {
            // Add the class "x" followed by num to the tile element.
            tile.classList.add("x" + num.toString());
        } else {
            // Add the class "x8192" to the tile element.
            tile.classList.add("x8192");
        }
    }
}

function updateBoard(uboard) {
    //Iterate over each element in uboard matrix and update corresponding tiles
    uboard.forEach(function(row, r) {
      row.forEach(function(_, c) {
        var tile = document.getElementById(r.toString() + "-" + c.toString());
        var num = uboard[r][c];
        updateTile(tile, num);
      });
    });
  }

function setGame() {
    board.forEach(function(row, r) {
      row.forEach(function(_, c) {
        // Create a new div element
        var tile = document.createElement("div");
        // Assign an ID to the div based on the row and column numbers
        tile.id = r.toString() + "-" + c.toString();
        // Get value from board array corresponding to current row and column
        var num = board[r][c];
        // Call updateTile function to update appearance or content of tile
        updateTile(tile, num);
        // Get container element with ID "board" and append tile element to it
        document.getElementById("board").append(tile);
      });
    });
    // Create 2 to begin the game
    board = Game_module.setTwo(board);
    updateBoard(board);
}

var value;
document.addEventListener("keyup", function(e) {
    if (e.code === "ArrowLeft") {
        value = Game_module.slideLeft(board);
        board = value.uboard;
        score += value.score;
        board = Game_module.setTwo(board);
    }else if (e.code === "ArrowRight") {
        value = Game_module.slideRight(board);
        board = value.uboard;
        score += value.score;
        board = Game_module.setTwo(board);
    }
    else if (e.code === "ArrowUp") {
        value = Game_module.slideUp(board);
        board = value.uboard;
        score += value.score;
        board = Game_module.setTwo(board);
    }
    else if (e.code === "ArrowDown") {
        value = Game_module.slideDown(board);
        board = value.uboard;
        score += value.score;
        board = Game_module.setTwo(board);
    }
    updateBoard(board);
    console.log(score);
    document.getElementById("score").innerText = score;
});