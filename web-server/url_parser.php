<?php

//mkdir() does not set the permissions specified. Must use chmod() afterwards

$rootdir="/var/www/html/trainingplanner/";
$scriptsdir=$rootdir . "scripts/";
$htpasswd_file=$scriptsdir . ".passwds";
$coaches_file=$rootdir . "admin/coaches";
$htpasswd="/usr/bin/htpasswd"; //use fullpath

function print_r2($val){
        echo '<pre>';
        print_r($val);
        echo  '</pre>';
}

function erasedir($path) {
    $dir = opendir($path);
    while( false !== ($file = readdir($dir)) ) {
        if (( $file != '.' ) && ( $file != '..' )) {
            $full = $path . '/' . $file;
            if (is_dir($full))
                erasedir($full);
            else if (is_file($full))
                unlink($full);
        }
    }
    closedir($dir);
    return rmdir($path);
}

// Function to verify credentials against .htpasswd file
function verify_credentials($username, $password, $htpasswd_file) {
    if (!file_exists($htpasswd_file)) {
        die("htpasswd file not found\r\n");
    }

    #print_r2("Authenticating " . $username . " with password " . $password);
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
    #print_r2($_REQUEST);
    #print_r2(getallheaders());
    if ($_SERVER['REQUEST_METHOD'] === 'POST') {
        // Check if the file was uploaded
        if (isset($_FILES['file']) && $_FILES['file']['error'] === UPLOAD_ERR_OK) {
            // Get file details
            $fileTmpPath = $_FILES['file']['tmp_name'];
            $fileName = $_FILES['file']['name'];
            $uploadFilePath = $uploadDir . "/" . basename($fileName);
            // Move the uploaded file to the upload directory
            if (move_uploaded_file($fileTmpPath, $uploadFilePath)) {
                chmod($uploadFilePath, 0664);
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
        if (substr("filename.txt",-4) == ".txt")
            header('Content-Type: text/plain');
        else
            header('Content-Type: application/octet-stream');
        header('Content-Disposition: attachment; filename="' . basename($filename) . '"');
        header('Expires: 0');
        header('Cache-Control: must-revalidate');
        header('Pragma: public');
        header('Content-Length: ' . filesize($filename));
        readfile($filename);
        #only return the contents of the file. Any extra string will only get in the way
        echo "Return code 0## ", basename($filename) . "##";
        return true;
    }
    echo "Return code 1 File not found: ", basename($filename);
    return false;
}

function get_binfile($binfile, $targetuser) {
    $src_dir = $rootdir . $targetuser;
    if (is_dir($src_dir)) {
        $files = array_values(array_diff(scandir($src_dir), array('.', '..')));
        foreach ($files as &$file) {
            $filename = basename($file);
            if ($binfile == substr($filename, 0, strlen($filename) - 4)) {
                download_file($src_dir, $filename);
                return;
            }
        }
    }
    echo "Return code 1 File not found: ", $binfile . " in " . $src_dir;
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
        chmod($coaches_file, 0664);
    }
    else {
        $coaches = file($coaches_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
        foreach ($coaches as $line) {
            if ($line == $coach) {
                echo "Return code 11: Coach already in the public list.\r\n";
                return;
            }
        }
        $fh = fopen($coaches_file, "a+")  or die("Return code: 10 Unable to open coaches file!" .$coaches_file . "\r\n");
    }
    fwrite($fh, $coach . "\n");
    fclose($fh);
    echo "Return code: 0 Coach in the public coaches file.\r\n";
}

function del_coach($coach) {
    global $coaches_file;
    if (file_exists($coaches_file)) {
        $coaches = file($coaches_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
        foreach ($coaches as $line) {
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

function get_coaches() {
    global $coaches_file;
    if (file_exists($coaches_file)) {
        $coaches = file($coaches_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
        foreach ($coaches as $coach) {
            echo $coach . " ";
        }
    }
    else
        echo "Return code 12 Public coaches file does not exist";
}

function request_coach($username, $coach) {
    global $rootdir;
    $user_info_file = $rootdir . $username . "/profile.txt";
    $requests_dir = $rootdir . $coach . "/requests/";
    if (!is_dir($requests_dir)) {
        if (mkdir($requests_dir, 0775, true))
            chmod($requests_dir, 0775);
        else {
            echo "Return code 13 Could not create requests dir";
            return false;
        }
    }
    $coach_request_file = $requests_dir . $user_name . ".txt";
    if (file_exists($coach_request_file))
        unlink($coach_request_file);
    if (copy($user_info_file, $coach_request_file)) {
        echo "Return code 0 Client request to coach successful";
        return;
    }
    echo "Return code 14 Could not complete client request to coach";
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

function run_dbscript($cmd, $cmd_opt, $userid) {
    global $scriptsdir;
    $dbscript=$scriptsdir . "usersdb.sh";

    ob_start();
    if ($userid) {
        if ($cmd_opt)
            passthru("$dbscript $userid $cmd $cmd_opt", $return_var);
        else
            passthru("$dbscript $userid $cmd", $return_var);
    }
    else {
        if ($cmd_opt)
            passthru("$dbscript $cmd $cmd_opt", $return_var);
        else
            passthru("$dbscript $cmd", $return_var);
    }
    $output = ob_get_clean();
    echo "Return code " . $return_var . "  " . $output;
    return $return_var;
}

$username = isset($_GET['user']) ? $_GET['user'] : '';

if ($username) { //regular, most common usage: download/upload file/info from/to server
    #echo "User name OK\r\n";
    $password = isset($_GET['password']) ? $_GET['password'] : '';
    if (!$password)
        die("Missing password\r\n");
    else {
        if (verify_credentials($username, $password, $htpasswd_file)) {
            // Authentication successful
            #echo "Authentication Successful! Welcome ", $username;
            #echo "\r\n";

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
            if (isset($_GET['getcoaches'])) {
                get_coaches();
                exit;
            }
            if (isset($_GET['requestcoach'])) {
                $coach = $_GET['requestcoach'];
                if ($coach)
                    request_coach($username, $coach);
                exit;
            }
            if (isset($_GET['upload'])) {
                $otheruser = $_GET['upload'];
                if ($otheruser) {
                    $fileDir=$rootdir . $otheruser;
                }
                upload_file($fileDir);
                exit;
            }
            if (isset($_GET['file'])) {
                if (isset($_GET['fromuser']))
                    $filedir = $rootdir .  $_GET['fromuser'];
                else
                    $filedir = $rootdir . $username;
                download_file($_GET['file'], $fileDir);
                exit;
            }
            if (isset($_GET['getbinfile'])) {
                $binfile = $_GET['getbinfile'];
                if ($binfile) {
                    $targetuser = isset($_GET['fromuser']) ? $_GET['fromuser'] : '';
                    if ($targetuser) {
                        get_binfile($binfile, $targetuser);
                        exit;
                    }
                }
            }

            echo "Missing action argument\r\n";
        }
        else {
            // Authentication failed
            header('HTTP/1.1 401 Unauthorized');
            echo "Authentication Failed. Invalid user or password.\r\n";;
        }
    }
}
else { //user management

    $query = isset($_GET['onlineuser']) ? $_GET['onlineuser'] : '';
    if ($query) { //Check if there is an already existing user in the online database. The  unique key used to identify an user is decided on the TrainingPlanner app source code. This script is agnostic to it
        run_dbscript("getid", $query, "");
        exit;
    }

    $userid = isset($_GET['onlinedata']) ? $_GET['onlinedata'] : '';
    if ($userid) { //Check if there is an already existing user in the online database. The  unique key used to identify an user is decided on the TrainingPlanner app source code. This script is agnostic to it
        run_dbscript("getall", "", $userid);
        exit;
    }

    $userid = isset($_GET['alteronlineuser']) ? $_GET['alteronlineuser'] : '';
    if ($userid) { //dbscript expects the file user.data to have been previously uploaded to $userid dir
        run_dbscript("add", "", $userid);
        exit;
    }

    run_dbscript("del", "", $username);
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
        $ok = run_htpasswd("-bd5", $username, $new_user_password);
        if ($ok == 0) {
            $userdir = $rootdir . $username;
            if (!is_dir($userdir)) {
                if (!mkdir($userdir, 0775))
                    $ok = 1;
                else
                    chmod($userdir, 0775);
             }
        }
        if ($ok == 0)
            echo "Return code: 0 User successfully created\r\n";
        else
            echo "Return code: 30 Error creating user\r\n";
        exit;
    }

    $username = isset($_GET['deluser']) ? $_GET['deluser'] : '';
    if ($username) { //remove user and their dir
        $ok = run_htpasswd("-D", $username, "");
        if ($ok == 0) {
            run_dbscript("del", "", $username);
            $userdir = $rootdir . $username;
            if (is_dir($userdir)) {
                if (!erasedir($userdir))
                    $ok = 1;
            }
        }
        if ($ok == 0)
            echo "Return code: 0 User successfully removed\r\n";
        else
            echo "Return code: 31 User removal failed\r\n";
        exit;
    }

    $username = isset($_GET['moduser']) ? $_GET['moduser'] : '';
    if ($username) { //remove moduser, create newuser and rename moduser dir to newuser
        $cmd_args = "-D";
        $ok = run_htpasswd("-D", $username, "");
        if ($ok == 0) {
            $new_username = isset($_GET['newuser']) ? $_GET['newuser'] : '';
            $new_password = isset($_GET['password']) ? $_GET['password'] : '';
            $ok = run_htpasswd("-bd5", $new_username, $new_password);
            if ($ok == 0) {
                $userdir = $rootdir . $username;
                if (is_dir($userdir)) {
                    if (!rename($rootdir . $username, $rootdir . $new_username))
                        $ok = 1;
                }
                else {
                    if (!mkdir($userdir, 0775))
                        $ok = 1;
                    else
                        chmod($userdir, 0775);
                }
            }
        }
        if ($ok == 0)
            echo "Return code: 0 User successfully modified\r\n";
        else
            echo "Return code: 32 User modification failed\r\n";
        exit;
    }

    print_r2("Welcome to the TrainingPlanner app server!");
}

?>
