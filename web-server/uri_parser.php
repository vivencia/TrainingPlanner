<?php

$rootdir="/var/www/html/trainingplanner/";
$htpasswd_file = $rootdir . "scripts/.passwds";
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

function run_htpasswd($cmd_args, $username, $password) {
    global $htpasswd;
    global $htpasswd_file;

     if (!file_exists($htpasswd_file))
        $cmd_args = $cmd_args . "c";
    $return_var = 0;
    $output = [];
    exec("$htpasswd $cmd_args $htpasswd_file $username $password", $output, $return_var);
    #echo "$htpasswd $cmd_args $htpasswd_file $username $password          Return code: $return_var ";
    return $return_var;
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
    $return_var = 0;
    $error_string = "";

    $username = isset($_GET['checkuser']) ? $_GET['checkuser'] : '';
    if ($username) { //check if user exists
        $user_password = isset($_GET['password']) ? $_GET['password'] : '';
        $return_var = run_htpasswd("-bv", $username, $user_password);
        switch ($return_var) {
            case 0: $error_string = "User exists and password is correct"; break;
            case 3: $error_string = "User exists and password is wrong"; break;
            case 6: $error_string = "User does not exist"; break;
            default: $error_string = "User does not exist"; break;
        }
        echo "Return code: $return_var $error_string";
        exit;
    }
    else {
        $username = isset($_GET['adduser']) ? $_GET['adduser'] : '';
        if ($username) { //new user creation. Encrypt password onto file and create the user's dir
            $new_user_password = isset($_GET['password']) ? $_GET['password'] : '';
            $return_var = run_htpasswd("-bd5", $username, $new_user_password);
            if ($return_var == 0) {
                $mkdir = "/usr/bin/mkdir";
                #echo "$mkdir $rootdir$username\r\n";
                exec("$mkdir $rootdir$username", $output, $return_var);
                $error_string  = "User successfully created";
                //print_r($output);
            }
            if ($return_var != 0)
                $error_string = "Error creating user";
            echo "Return code: $return_var $error_string";
            exit;
        }
        else {
            $username = isset($_GET['deluser']) ? $_GET['deluser'] : '';
            if ($username) { //remove user and their dir
                $return_var = run_htpasswd("-D", $username, "");
                if ($return_var == 0) {
                    $rmdir = "/usr/bin/rmdir";
                    #echo "$rmdir $rootdir$username\r\n";
                    exec("$rmdir $rootdir$username", $output, $return_var);
                    $error_string = "User successfully removed";
                }
                if ($return_var != 0)
                    $error_string = "User removal failed";
                echo "Return code: $return_var $error_string";
                exit;
            }
            else {
                $username = isset($_GET['moduser']) ? $_GET['moduser'] : '';
                if ($username) { //remove moduser, create newuser and rename moduser dir to newuser
                    $cmd_args = "-D";
                    $return_var = run_htpasswd("-D", $username, "");
                    if ($return_var == 0) {
                        $new_username = isset($_GET['newuser']) ? $_GET['newuser'] : '';
                        $new_password = isset($_GET['password']) ? $_GET['password'] : '';
                        $return_var = run_htpasswd("-bd5", $new_username, $new_password);
                        if ($return_var == 0) {
                            $mvdir = "/usr/bin/mv";
                            #echo "$mvdir $rootdir$username $rootdir$new_username\r\n";
                            exec("$mvdir $rootdir$username $rootdir$new_username", $output, $return_var);
                            $error_string = "User successfully modified";
                        }
                    }
                    if ($return_var != 0)
                        $error_string = "User modification failed";
                    echo "Return code: $return_var $error_string";
                    exit;
                }
            }
        }
    }
    die("Missing user or command");
}

?>
