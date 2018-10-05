function validate() {
  if (document.getElementById("e").selectedIndex == 0) {
    alert('Please select an event.');
    return false;
  } else {
    return true;
  }
}    
