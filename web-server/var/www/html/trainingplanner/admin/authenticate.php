<?php
// Path to your .htpasswd file
$rootdir="/var/www/html/trainingplanner/";
$htpasswd_file = $rootdir . "admin/.passwds";
$htpasswd = "/usr/bin/htpasswd"; //use fullpath

// Function to verify credentials against .htpasswd file
function verify_credentials($username, $password, $htpasswd_file) {
    if (!file_exists($htpasswd_file)) {
        die("htpasswd file not found");
    }

    // Read the .htpasswd file line by line
    $lines = file($htpasswd_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
    foreach ($lines as $line) {
        // Each line is in format: username:hashed_password
        list($storedUser, $storedHash) = explode(':', $line, 2);

        if ($storedUser === $username) {
            // Check password against stored hash
            if (password_verify($password, $storedHash) || crypt($password, $storedHash) === $storedHash) {
                return true;
            }
        }
    }
    return false;
}

function download_file($file) {
    if (file_exists($file)) {
        header('Content-Description: File Transfer');
        header('Content-Type: application/octet-stream');
        header('Content-Disposition: attachment; filename="' . basename($file) . '"');
        header('Expires: 0');
        header('Cache-Control: must-revalidate');
        header('Pragma: public');
        header('Content-Length: ' . filesize($file));
        readfile($file);
        return true;
    }
    return false;
}

$username = isset($_GET['user']) ? $_GET['user'] : '';

if ($username) { //regular, most common usage: download/upload file/info from/to server
    echo "User name OK\r\n";
    $password = isset($_GET['password']) ? $_GET['password'] : '';
    if (!$password)
        die("Missing password");
    else {
        if (verify_credentials($username, $password, $htpasswd_file)) {
            // Authentication successful
            echo "Authentication Successful! Welcome ", $username;
            $filename = isset($_GET['file']) ? $rootdir . $_GET['file'] : '';
            if (download_file($filename)) {
                echo "File found: ", $filename;
                exit;
            }
            else
                echo "File not found: ", $filename;
        }
        else {
            // Authentication failed
            header('HTTP/1.1 401 Unauthorized');
            echo 'Authentication Failed. Invalid user or password.';
        }
    }
}
else { //other commands to server
    $username = isset($_GET['adduser']) ? $_GET['adduser'] : '';
    if ($username) { //new user creation. Encrypt password onto file and create the user's dir
        $new_user_password = isset($_GET['password']) ? $_GET['password'] : '';
        $cmd_args = "-bd5";
        $return_var = 0;
        $output = [];
        echo "$htpasswd $cmd_args $htpasswd_file $username $new_user_password";
        exec("$htpasswd $cmd_args $htpasswd_file $username $new_user_password", $output, $return_var);
        if ($return_var == 0) {
            $mkdir = "/usr/bin/mkdir";
            echo "$mkdir $rootdir$username\r\n";
            exec("$mkdir $rootdir$username", null, $return_var);
            echo "User successfully created\r\n";
            //print_r($output);
        }
        if ($return_var != 0)
            echo "Error creating user\r\n";
        echo "\nExit Code: $return_var";
    }
    else
        die("Missing user or command");
}

?>
