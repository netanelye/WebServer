var tableSize = 10;
var round = 1;
var black = 2;
var white = 2;
var blackScore2 = 0;
var whiteScore2 = 0;
var gameTimeStart = 0;
var startTimePlayer = 0;
var sumBlackTime = 0;
var sumWhiteTime = 0;
var playerTurn = "dotBlack";
var scoreTable = document.getElementById("score");
var gameIsOn = false;

function openMenu() {
    document.getElementById("myDropdown").classList.toggle("show");
}

function quit() {

    if (playerTurn == "dotBlack") {
        document.getElementById("winnerBar").innerHTML = "White IS THE WINNER WITH " + white + " Points";
        appendStatistics();
    }
    else {
        document.getElementById("winnerBar").innerHTML = "BLACK IS THE WINNER WITH " + black + " Points";
        appendStatistics();
    }

   document.getElementById("x").onclick = closeTab;
    openMainMenu();
}

window.onclick = function (event) {
    if (!event.target.matches('#dropbtn')) {
        var dropdowns = document.getElementsByClassName("dropdown-content");
        var i;
        for (i = 0; i < dropdowns.length; i++) {
            var openDropdown = dropdowns[i];
            if (openDropdown.classList.contains('show')) {
                openDropdown.classList.remove('show');
            }
        }
    }
}

function changeTo(number) {
    document.getElementById("dropbtn").innerHTML = number;
    tableSize = number;
    var dropdowns = document.getElementsByClassName("dropdown-content");
    var i;
    for (i = 0; i < dropdowns.length; i++) {
        var openDropdown = dropdowns[i];
        if (openDropdown.classList.contains('show')) {
            openDropdown.classList.remove('show');
        }
    }
}

function hereWeGo() {
    var size = document.getElementById("dropbtn").innerHTML;
    document.getElementById("start").innerHTML = "new game";
    document.getElementById("x").onclick = closeMenu;
    closeMenu();
    startGame(size);
}

function startGame(tableSize) 
{
    var gameTable = document.getElementById("gtable");
    blackScore2 = 0;
    whiteScore2 = 0;
    gameTimeStart = 0;
    startTimePlayer = 0;
    sumBlackTime = 0;
    sumWhiteTime = 0;
    black = 2;
    white = 2;
    round = 1;

    playerTurn = "dotBlack";
    var x = tableSize;
    var t = new Date();
    startTimePlayer = t.getMinutes() * 60 + t.getSeconds();
    gameTimeStart = startTimePlayer;
    updateScores();
    if (gameIsOn == true) {
        
        while (gameTable.firstChild) {
            gameTable.removeChild(gameTable.firstChild);

            document.getElementById("roundCount").innerHTML = "Round number " + round;
        }
        document.getElementById("winnerBar").innerHTML = "I AM THE BEST GAME EVER!!";
    }
    else {
        var menuBtn = document.createElement("BUTTON");
        var quitBtn = document.createElement("BUTTON");
        menuBtn.innerHTML = "Main Menu";
        quitBtn.innerHTML = "Quit";
        menuBtn.id = "mainMenu";
        quitBtn.id = "quit";
        document.getElementById("menuButton").appendChild(menuBtn);
        document.getElementById("quitButton").appendChild(quitBtn);
        document.getElementById("mainMenu").onclick = openMainMenu;
        document.getElementById("quitButton").onclick = quit;


    }


    gameIsOn = true;
    for (var i = 0; i < x; i++) {
        var row = document.createElement("tr");
        row.className = "row";
        document.getElementById("roundCount").innerHTML = "Round number " + round;


        for (var j = 0; j < x; j++) {
            var col = document.createElement("td");
            col.className = "col";
            col.id = (i * tableSize) + j;
            col.onclick = function () { HasBeenClicked(this.id) };
            if ((j + i) % 2) {
                col.style.background = "gray";
            }
            else {
                col.style.background = "pink";
            }

            if ((i == x / 2 - 1 && j == x / 2 - 1) || (i == x / 2 && j == x / 2)) {
                var blackChecker = document.createElement("div");
                blackChecker.className += "dotBlack";
                col.appendChild(blackChecker);
            }
            if ((i == x / 2 && j == x / 2 - 1) || (i == x / 2 - 1 && j == x / 2)) {
                var whiteChecker = document.createElement("div");
                whiteChecker.className += "dotWhite";
                col.appendChild(whiteChecker);
            }
            row.append(col);
        }

        gameTable.append(row);

    }

    document.getElementById("x").onclick = closeMenu;
    makeBulge();

}

function HasBeenClicked(index) {
    var TableTab = document.getElementById(index);
    var attackerColor = playerTurn;
    var inX = parseInt(index);
    var inY = 0;

    if (inX > tableSize - 1) {
        inY = parseInt(inX / tableSize);
        inX = inX % tableSize;
    }

    var canBeClicked = checkIfCanBeClicked(inX, inY);
    if (canBeClicked) {
        if (TableTab.childElementCount == 0)//if there is no children 
        {

            updatePlayerTime();
            round++;

            if (playerTurn == "dotBlack") {
                var blackChecker = document.createElement("div");
                blackChecker.className = "dotBlack";
                black++;
                TableTab.appendChild(blackChecker);
                playerTurn = "dotWhite";
                document.getElementById("player").innerHTML = "White player's turn";
            }
            else {
                var whiteChecker = document.createElement("div");
                whiteChecker.className = "dotWhite";
                white++;
                TableTab.appendChild(whiteChecker);
                playerTurn = "dotBlack";
                document.getElementById("player").innerHTML = "Black player's turn";
            }


            fixBoard(index, attackerColor, playerTurn);//player changed already to the attacked player
            updateScores();
            makeBulge();

        }
    }
    else {
        alert("You cant place a piece here, please place the piece on the flashing tiles")
    }
}

function updatePlayerTime() {
    var t = new Date();
    var times = t.getMinutes() * 60 + t.getSeconds();
    if (playerTurn == "dotBlack") {
        sumBlackTime += times - startTimePlayer;

    }
    else {
        sumWhiteTime += times - startTimePlayer;
    }
    startTimePlayer = times;
}

function appendStatistics() {
    round--;
    var son = document.getElementById("score").cloneNode(true);
    son.firstElementChild.removeChild(son.firstElementChild.firstChild);
    document.getElementById("winnerBar").appendChild(son);
}

function thereIsWinner() {

    if (black > white) {
        document.getElementById("winnerBar").innerHTML = "BLACK IS THE WINNER WITH " + black + " Points";
        appendStatistics();
    }
    else if (black < white) {
        document.getElementById("winnerBar").innerHTML = "WHITE IS THE WINNER WITH " + white + " Points";
        appendStatistics();
    }
    else {
        document.getElementById("winnerBar").innerHTML = "DRAW!!!!";
        appendStatistics();
    }
    document.getElementById("x").onclick = closeTab;
    openMainMenu();

}

function duplicate(src) {
    return Object.assign({}, src);

}

function fixBoard(clickedId, attackerColor, attackedColor) {
    var inX = parseInt(clickedId);
    var inY = 0;

    if (inX > tableSize - 1) {
        inY = parseInt(inX / tableSize);
        inX = inX % tableSize;
    }

    for (var i = -1; i <= 1; i++) {
        for (var j = -1; j <= 1; j++) {
            if (j != 0 || i != 0) {
                vectorChecker(inX, inY, i, j, attackerColor, attackedColor, clickedId);
            }
        }
    }
}

function vectorChecker(inX, inY, vecX, vecY, attackerColor, attackedColor, clickedId) {
    inX += vecX;
    inY += vecY;
    tabContain = attackedColor;
    var i = 0;
    var changeArr = [];

    while (inX < tableSize && inX > -1 && inY < tableSize && inY > -1 && tabContain == attackedColor) {
        tabContain = checkWhatContain(inY * tableSize + inX);

        if (tabContain == attackedColor) {
            changeArr[i] = inY * tableSize + inX;
            i++;
        }
        else if (tabContain == attackerColor) {
            ChangeColor(changeArr, i, attackerColor, attackedColor);
        }
        inX += vecX;
        inY += vecY;
    }
}

function checkWhatContain(XY) {
    var TableTab = document.getElementById(XY);

    if (TableTab.childElementCount == 0) {
        return "empty";
    }
    else {
        if (TableTab.firstChild.className == "dotWhite") {
            return "dotWhite";
        }
        else
            return "dotBlack";
    }

}

function ChangeColor(changeArr, size, attackerColor, attackedColor) {

    for (var i = 0; i < size; i++) {
        var TableTab = document.getElementById(changeArr[i]);
        var Checker = document.createElement("div");
        Checker.className = attackerColor;
        TableTab.replaceChild(Checker, TableTab.firstChild);

        if (attackedColor == "dotBlack") {
            black--;
            white++;
        }
        else {
            black++;
            white--;
        }
    }
}
function updateScores() {
    document.getElementById("blackScore").innerHTML = "Black score " + black;
    document.getElementById("whiteScore").innerHTML = "White score " + white;
    if (white + black > 5) {
        if (black == 2)
            blackScore2++;
        if (white == 2)
            whiteScore2++;
    }
    document.getElementById("blackScore2").innerHTML = "Black Score 2 : " + blackScore2;
    document.getElementById("whiteScore2").innerHTML = "White Score 2 : " + whiteScore2;

    if (white + black > 4) {
        if (playerTurn == "dotWhite") {
            document.getElementById('avregeTimeBlack').innerHTML = "TIME Black: " + parseInt((sumBlackTime / 60) / ((round) / 2)) + ":" + checkTime(parseInt((sumBlackTime % 60) / ((round + 1) / 2)));
        }
        else {
            document.getElementById('avregeTimeWhite').innerHTML = "TIME White: " + parseInt((sumWhiteTime / 60) / ((round - 1) / 2)) + ":" + checkTime(parseInt((sumWhiteTime % 60) / ((round - 1) / 2)));
        }
    }
    else {
        document.getElementById('avregeTimeBlack').innerHTML = "TIME Black: ";
        document.getElementById('avregeTimeWhite').innerHTML = "TIME White: ";
    }

    if (white + black == tableSize * tableSize || black == 0 || white == 0) {
        thereIsWinner();
    }

    document.getElementById("roundCount").innerHTML = "Round number " + round;
}

function makeBulge() {
    var boardIndex = 0;
    var elementInBoard;
    for (var i = 0; i < tableSize; i++) {

        for (var j = 0; j < tableSize; j++) {
            elementInBoard = document.getElementById(boardIndex.toString());


            if (checkIfCanBeClicked(j, i) && elementInBoard.childElementCount == 0) {
                elementInBoard.classList += " pulse";
            }
            else {
                if (elementInBoard.childElementCount == 1) {
                    elementInBoard.classList.remove("pulse");
                }

            }
            if (elementInBoard.classList.contains("pulse")) {
                elementInBoard.classList.remove("pulse");
                elementInBoard.classList += " pulse";
            }

            boardIndex++;
        }


    }
}

function startTime() {
    var now = new Date();
    var curTime;
    var m = now.getMinutes();
    var s = now.getSeconds();
    curTime = m * 60 + s - gameTimeStart;
    m = parseInt(curTime / 60);
    s = curTime % 60;

    s = checkTime(s);
    document.getElementById('time').innerHTML = "TIME: " + m + ":" + s;
    var t = setTimeout(startTime, 500);
}

function checkTime(i) {
    if (i < 10) { i = "0" + i };
    return i;
}
function closeTab() {
    window.close();
}

function closeMenu() {
    document.getElementById("mymenu").style.display = "none";
}

function openMainMenu() {
    document.getElementById("mymenu").style.display = "block";
}
function checkIfCanBeClicked(inX, inY) {

    for (var i = -1; i <= 1; i++) {
        for (var j = -1; j <= 1; j++) {
            if ((i != 0 || j != 0) && inX + j < tableSize && inX + j > -1 && inY + i > -1 && inY + i < tableSize) {
                if (checkWhatContain((inY + i) * tableSize + inX + j) != "empty")
                    return true;
            }

        }
    }

    return false;
}