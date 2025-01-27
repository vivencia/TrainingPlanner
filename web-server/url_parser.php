<?php

$rootdir="/var/www/html/trainingplanner/";
$htpasswd_file=$rootdir . "scripts/.passwds";
$coaches_file=$rootdir . "scripts/coaches";
$htpasswd="/usr/bin/htpasswd"; //use fullpath

// Function to verify credentials against .htpasswd file
function verify_credentials($username, $password, $htpasswd_file) {
    if (!file_exists($htpasswd_file)) {
        die("htpasswd file not found\r\n");
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

function upload_file($uploadDir) {
    if ($_SERVER['REQUEST_METHOD'] === 'POST') {
        // Check if the file was uploaded
        if (isset($_FILES['file']) && $_FILES['file']['error'] === UPLOAD_ERR_OK) {
            // Get file details
            $fileTmpPath = $_FILES['file']['tmp_name'];
            $fileName = $_FILES['file']['name'];
            $uploadPath = $uploadDir . "/" . basename($fileName);
            // Move the uploaded file to the upload directory
            if (move_uploaded_file($fileTmpPath, $uploadPath)) {
                echo "Return code 0 File uploaded successfully: " . htmlspecialchars($fileName);
                echo "\r\n";
                return true;
            }
            else {
                echo "Return code 20 Failed to move the uploaded file.\r\n";
            }
        }
        else {
            echo $_FILES['file'];
            echo "Return code 21 No file uploaded or an error occurred.\r\n";
        }
    }
    else {
        echo "Return code 22 Invalid request method.\r\n";
    }
    return false;
}

function download_file($file,$downloadDir) {
    $filename=$downloadDir . "/" . $file;
    if (file_exists($filename)) {
        header('Content-Description: File Transfer');
        header('Content-Type: application/octet-stream');
        header('Content-Disposition: attachment; filename="' . basename($filename) . '"');
        header('Expires: 0');
        header('Cache-Control: must-revalidate');
        header('Pragma: public');
        header('Content-Length: ' . filesize($filename));
        readfile($filename);
        echo "Return code 0 File found: ", $filename;
        return true;
    }
    echo "Return code 1 File not found: ", $filename;
    return false;
}

function scan_dir($path) {
    $files = array_values(array_diff(scandir($path), array('.', '..')));
    foreach ($files as &$file) {
        echo $file . "\r\n";
    }
}

function add_coach($coach) {
    global $coaches_file;
    if (!file_exists($coaches_file)) {
        $fh = fopen($coaches_file, "w")  or die("Return code: 10 Unable to open coaches file!" .$coaches_file . "\r\n");
    }
    else {
        $coaches = file($coaches_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
        foreach ($coaches as  $line) {
            if ($line == $coach) {
                echo "Return code 11: Coach already in the public list.\r\n";
                return;
            }
        }
        $fh = fopen($coaches_file, "a+")  or die("Return code: 10 Unable to open coaches file!" .$coaches_file . "\r\n");
    }
    fwrite($fh, $coach . "\n");
    fclose($fh);
    echo "Return code: 0 Coache in the public coaches file.\r\n";
}

function del_coach($coach) {
    global $coaches_file;
    if (file_exists($coaches_file)) {
        $coaches = file($coaches_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
        foreach ($coaches as  $line) {
            if ($line != $coach) {
                $new_coaches = $new_coaches . $line . "\r\n";
            }
        }
        $fh = fopen($coaches_file, "w")  or die("Return code: 10 Unable to open coaches file!" .$coaches_file . "\r\n");
        fwrite($fh, $new_coaches);
        fclose($fh);
        echo "Return code: 0 Coach removed from the public coaches file.\r\n";
    }
    else
        echo "Return code 12 Public coaches file does not exist";
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
        die("Missing password\r\n");
    else {
        if (verify_credentials($username, $password, $htpasswd_file)) {
            // Authentication successful
            echo "Authentication Successful! Welcome ", $username;
            echo "\r\n";

            $fileDir=$rootdir . $username;

            if (isset($_GET['listfiles'])) {
                scan_dir($fileDir);
                exit;
            }
            if (isset($_GET['addcoach'])) {
                add_coach($username);
                exit;
            }
            if (isset($_GET['delcoach'])) {
                del_coach($username);
                exit;
            }
            if (isset($_GET['upload'])) {
                $filename = $rootdir . $_GET['upload'];
                if ($filename) {
                    upload_file($fileDir);
                    exit;
                }
            }
            if (isset($_GET['file'])) {
                $filename = $rootdir . $_GET['file'];
                if ($filename) {
                    $filename=basename($filename);
                    download_file($filename,$fileDir);
                    exit;
                }
            }

            echo "Missing filename name argument(either upload= or file=)\r\n";
        }
        else {
            // Authentication failed
            header('HTTP/1.1 401 Unauthorized');
            echo "Authentication Failed. Invalid user or password.\r\n";;
        }
    }
}
else { //user management
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
        echo "Return code: $return_var $error_string\r\n";
        exit;
    }

    $username = isset($_GET['adduser']) ? $_GET['adduser'] : '';
    if ($username) { //new user creation. Encrypt password onto file and create the user's dir
        $new_user_password = isset($_GET['password']) ? $_GET['password'] : '';
        $return_var = run_htpasswd("-bd5", $username, $new_user_password);
        if ($return_var == 0) {
            $mkdir = "/usr/bin/mkdir";
            #echo "$mkdir $rootdir$username\r\n";
            exec("$mkdir $rootdir$username", $output, $return_var);
            $error_string  = "User successfully created\r\n";
            //print_r($output);
        }
        if ($return_var != 0)
            $error_string = "Error creating user\r\n";
        echo "Return code: $return_var $error_string\r\n";
        exit;
    }

    $username = isset($_GET['deluser']) ? $_GET['deluser'] : '';
    if ($username) { //remove user and their dir
        $return_var = run_htpasswd("-D", $username, "");
        if ($return_var == 0) {
            $rmdir = "/usr/bin/rmdir";
            #echo "$rmdir $rootdir$username\r\n";
            exec("$rmdir $rootdir$username", $output, $return_var);
            $error_string = "User successfully removed\r\n";
        }
        if ($return_var != 0)
            $error_string = "User removal failed\r\n";
        echo "Return code: $return_var $error_string\r\n";
        exit;
    }

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
                $error_string = "User successfully modified\r\n";
            }
        }
        if ($return_var != 0)
            $error_string = "User modification failed\r\n";
        echo "Return code: $return_var $error_string\r\n";
        exit;
    }
    die("Missing user or command\r\n");
}

?>
