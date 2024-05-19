import R from "./ramda.js";
/**
 * This is a module for playing 2048
 * @namespace Game_module
 */

const Game_module = Object.create(null);

/**
* Insert a new cell where empty
* @memberof Game_module
* @function init
* @param None
* @returns {Array} - Initial board as 2D array with all cells set to 0.
*/
Game_module.init = function () {
    return [[0,0,0,0],
            [0,0,0,0],
            [0,0,0,0],
            [0,0,0,0]
        ];
};

//function randomly generates a 2 or 4
function twoOrFour(){
    const randomNumber = Math.random();
     // Return 2 or 4 based on the random number
     if (randomNumber < 0.5) {return 2;} else {return 4;}
}
/**
*Move and combine cells
*@memberof Game_module
*@function setTwo
*@param {Array} uboard - The game board represented as a 2D array.
*@returns {Array} - The updated game board with a new cell (2) inserted.
*/
Game_module.setTwo = function (uboard) {
    if (!Game_module.hasEmptyTile(uboard)) {
        return uboard;
    }
    var rows = uboard.length;
    var columns = uboard[0].length;
    var found = false;
    // Generate an array of indices representing all possible positions
    var indices = Array.from({ length: rows }, function (_, r) {
        return Array.from({ length: columns }, function (_, c) {
            return [r, c];
        });
    }).flat();
    // Shuffle the indices array
    indices.sort(function () {
        return Math.random() - 0.5;
    });
    // Find the first empty position using forEach
    indices.forEach(function (index) {
        var r = index[0];
        var c = index[1];
        if (!found && uboard[r][c] === 0) {
            uboard[r][c] = twoOrFour();
            found = true;
        }
    });
    return uboard;
};

/**
Check if there is an empty tile on the game board.
@memberof Game_module
@function hasEmptyTile
@param {Array} uboard - The game board represented as a 2D array.
@returns {boolean} - Returns true if there is an empty tile, false otherwise.
*/
Game_module.hasEmptyTile = function(uboard) {
    var foundEmpty = false;
    uboard.forEach(function(row) {
        row.forEach(function(num) {
            if (num === 0) {
                foundEmpty = true;
            }
        });
    });
    return foundEmpty;
};
//removes zero values from a given array
function filterZero(row) {
  return row.filter(function(num) {
    return num !== 0;
  });
}

//moves and combines cells in a given row
function slide(row) {
    // Initialize a variable to keep track of the score
    var score = 0;
    row = filterZero(row);
    // Remove zeros from the row using filterZero function
    row.forEach(function(num, i) {
      // Iterate over each element in the row
      // Check if current element is equal to next element and not at last index
      if (i < row.length - 1 && row[i] === row[i + 1]) {
        // Update score by adding value of the current element multiplied by 2
        score += row[i] * 2;
        // Double the value of the current element
        row[i] *= 2;
        // Set the value of the next element to 0
        row[i + 1] = 0;
      }
    });
    row = filterZero(row); // Remove zeros from the row again
    const numZeroesToAdd = 4 - row.length;
    // Calculate the number of zeros to add to the row
    // Add the calculated number of zeros to the end of the row
    Array(numZeroesToAdd).fill(0).forEach(function() {
      row.push(0);
    });
    // Return an object containing the updated row and score
    return { row: row, score: score };
}

/**
*Move left
*@memberof Game_module
*@function slideLeft
*@param {Array} uboard - The game board represented as a 2D array.
*@returns {Object} - Object containing updated game board (uboard) and score.
*/
Game_module.slideLeft = function (uboard) {
    // Initialize a variable to keep track of the total score
    var score = 0;
    // Initialize a variable to keep track of the score obtained from each row
    var scored = 0;
    // Iterate over each row in the game board
    uboard.forEach(function(row, r) {
        // Declare a variable to store the result of the slide function
        var val;
        // Call the slide function to move and combine cells in the current row
        val = slide(row);
        // Update the row with the updated row returned from the slide function
        row = val.row;
        // Update the scored variable with score obtained from slide function
        scored = val.score;
        // Update the game board with the updated row
        uboard[r] = row;
        // Add the scored value to the total score
        score += scored;
    });
    // Return an object containing the updated game board and the total score
    return { uboard, score };
};


/**
*Move right
*@memberof Game_module
*@function slideRight
*@param {Array} uboard - The game board represented as a 2D array.
*@returns {Object} - Object containing updated game board (uboard) and score.
*/
Game_module.slideRight = function (uboard) {
    // let rows = uboard.length;
    var score = 0;
    var scored = 0;
    uboard.forEach(function(row, r) {
        row.reverse();
        var val;
        val = slide(row);
        row = val.row;
        scored = val.score;
        uboard[r] = row.reverse();
        score += scored;
    });
    return { uboard, score };
};

/**
*Move up
*@memberof Game_module
*@function slideUp
*@param {Array} uboard - The game board represented as a 2D array.
*@returns {Object} - Object containing the updated game board (uboard) score.
*/
Game_module.slideUp = function (uboard) {
    var score = 0;
    var scored = 0;
    uboard[0].forEach(function(_, c) {
        var row = [uboard[0][c], uboard[1][c], uboard[2][c], uboard[3][c]];
        var val = slide(row);
        row = val.row;
        scored = val.score;
        row.forEach(function(value, r) {
            uboard[r][c] = value;
        });
        score += scored;
    });
    return { uboard, score };
};

/**
*Move down
*@memberof Game_module
*@function slideDown
*@param {Array} uboard - The game board represented as a 2D array.
*@returns {Object} - Object containing updated game board (uboard) and score.
*/
Game_module.slideDown = function (uboard) {
    var score = 0;
    var scored = 0;
    // let columns = uboard[0].length;
    uboard[0].forEach(function (_, c) {
        var row = [uboard[0][c], uboard[1][c], uboard[2][c], uboard[3][c]];
        row.reverse();
        var val;
        val = slide(row);
        row = val.row.reverse();
        scored = val.score;
        row.forEach(function (value, r) {
            uboard[r][c] = value;
        });
        score += scored;
    });
    return { uboard, score };
};

export default Object.freeze(Game_module);