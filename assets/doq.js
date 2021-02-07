/* doq.js - 'doq' javascript boilerplate
 * 
 * @author: Cade Brown <cade@kscript.org>
 */

/* Toggles side navigation */
function doq_togglesidenav() {
    var x = document.getElementById("sidenav");
    if (x.style.display === "block") {
        x.style.display = "none";
    } else {
        x.style.display = "block";
    }
}

