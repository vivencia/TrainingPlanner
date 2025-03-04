<?php

//mkdir() does not set the permissions specified. Must use chmod() afterwards

$rootdir="/var/www/html/trainingplanner/";
$scriptsdir=$rootdir . "scripts/";
$htpasswd_file=$scriptsdir . ".passwds";
$coaches_file=$rootdir . "admin/coaches";
$htpasswd="/usr/bin/htpasswd"; //use fullpath

// set the default timezone to use.
date_default_timezone_set('America/Sao_Paulo');

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
                echo "Return code: 0 File uploaded successfully: " . htmlspecialchars($fileName);
                echo "\r\n";
                return true;
            }
            else {
                echo "Return code: 20 Failed to move the uploaded file.\r\n";
            }
        }
        else {
            echo $_FILES['file'];
            echo "Return code: 21 No file uploaded or an error occurred.\r\n";
        }
    }
    else {
        echo "Return code: 22 Invalid request method.\r\n";
    }
    return false;
}

function download_file($file, $downloadDir) {
    $filename=$downloadDir . "/" . $file;
    if (file_exists($filename)) {
        $file_name = basename($filename)."%%";
        $size = strlen($file_name) + filesize($filename);
        echo $file_name;
        header('Content-Description: File Transfer');
        if (substr($filename,-4) == ".txt")
            header('Content-Type: text/plain');
        else
            header('Content-Type: application/octet-stream');
        header('Content-Disposition: attachment; filename="' . basename($filename) . '"');
        header('Expires: 0');
        header('Cache-Control: must-revalidate');
        header('Pragma: public');
        header('Content-Length: ' . $size);
        readfile($filename);
        //only return the contents of the file. Any extra string will only get in the way
        return true;
    }
    echo "Return code: 1 File not found: ", $filename;
    return false;
}

function get_binfile($binfile, $targetuser) {
    global $rootdir;
    $src_dir = $rootdir . $targetuser;
    if (is_dir($src_dir)) {
        $files = array_values(array_diff(scandir($src_dir), array('.', '..')));
        foreach ($files as &$file) {
            $filename = basename($file);
            if ($binfile == substr($filename, 0, strlen($filename) - 4)) {
                download_file($filename, $src_dir);
                return;
            }
        }
    }
    echo "Return code: 1 File not found: ", $binfile . " in " . $src_dir;
}

function check_file_ctime($binfile, $targetuser) {
    global $rootdir;
    $filename = $rootdir . $targetuser . "/" . $binfile;
    if (is_file($filename))
        echo "Return code: 0 ", date('Hisymd', filectime($filename));
    else
        echo "Return code: 1 File not found:  ", $filename;
}

function scan_dir($path) {
    $files = array_values(array_diff(scandir($path), array('.', '..')));
    foreach ($files as &$file) {
        echo $file . " ";
    }
}

function add_coach($coach) {
    global $coaches_file;
    if (!file_exists($coaches_file)) {
        $fh = fopen($coaches_file, "w") or die("Return code: 10 Unable to create coaches file!" .$coaches_file . "\r\n");
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
        $fh = fopen($coaches_file, "a+") or die("Return code: 10 Unable to open coaches file!" .$coaches_file . "\r\n");
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
        $fh = fopen($coaches_file, "w") or die("Return code: 10 Unable to open coaches file!" .$coaches_file . "\r\n");
        fwrite($fh, $new_coaches);
        fclose($fh);
        echo "Return code: 0 Coach removed from the public coaches file.\r\n";
    }
    else
        echo "Return code: 12 Public coaches file does not exist";
}

function get_coaches() {
    global $coaches_file;
    if (file_exists($coaches_file)) {
        echo "Return code: 0 ";
        $coaches = file($coaches_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
        foreach ($coaches as $coach) {
            echo $coach . " ";
        }
    }
    else
        echo "Return code: 12 Public coaches file does not exist";
}

function request_coach($username, $coach) {
    global $rootdir;
    $requests_file = $rootdir . $coach . "/requests.txt";
    if (!file_exists($requests_file)) {
        $fh = fopen($requests_file, "w") or die("Return code: 10 Unable to create requests file!" .$requests_file . "\r\n");
        chmod($requests_file, 0664);
    }
    else {
        $clients = file($requests_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
        foreach ($clients as $line) {
            if ($line == $username) {
                echo "Return code 11: Client request already placed.\r\n";
                return;
            }
        }
        $fh = fopen($requests_file, "a+") or die("Return code: 10 Unable to open requests file to append new request!" .$requests_file . "\r\n");
    }
    fwrite($fh, $username . "\n");
    fclose($fh);
    echo "Return code: 0 Coach request OK.\r\n";
}

function delete_client_request($coach, $client) {
    global $rootdir;
    $requests_file = $rootdir . $coach . "/requests.txt";
    if (file_exists($requests_file)) {
        $clients = file($requests_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
        foreach ($clients as $line) {
            if ($line != $client) {
                $new_clients = $new_clients . $line . "\r\n";
            }
        }
        $fh = fopen($requests_file, "w") or die("Return code: 10 Unable to open requests file!" .$requests_file . "\r\n");
        fwrite($fh, $new_clients);
        fclose($fh);
        echo "Return code: 0 Client removed from the coach's request file file.\r\n";
        return true;
    }
    else {
        echo "Return code: 12 Coach's requests file does not exist";
        return false;
    }
}

function list_clients_requests($coach) {
    global $rootdir;
    $requests_file = $rootdir . $coach . "/requests.txt";
    if (file_exists($requests_file)) {
        echo "Return code: 0 ";
        $clients = file($requests_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
        foreach ($clients as $client) {
            echo $client . " ";
        }
    }
    else
        echo "Return code: 12 Requests file does not exist" . $requests_file;
}

function accept_client_request($coach, $client)
{
    if (delete_client_request($coach, $client)) {
        global $rootdir;
        $accepts_file = $rootdir . $client . "/coaches_accepted.txt";
        if (!file_exists($accepts_file)) {
            $fh = fopen($accepts_file, "w")  or die("Return code: 10 Unable to create accepts file!" .$accepts_file . "\r\n");
            chmod($accepts_file, 0664);
        }
        else {
            $coaches = file($accepts_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
            foreach ($coaches as $line) {
                if ($line == $coach) {
                    echo "Return code 11: Coach accept answer already placed.\r\n";
                    return;
                }
            }
            $fh = fopen($accepts_file, "a+")  or die("Return code: 10 Unable to open accepts file to append new anwer!" .$accepts_file . "\r\n");
        }
        fwrite($fh, $coach . "\n");
        fclose($fh);
        echo "Return code: 0 Coach's answer to client's request OK";
        return true;
    }
    echo "Return code: 11 Could not place coach's answer to client";
    return false;
}

function reject_client_request($coach, $client)
{
    if (delete_client_request($coach, $client)) {
        global $rootdir;
        $rejects_file = $rootdir . $client . "/coaches_rejected.txt";
        if (!file_exists($rejects_file)) {
            $fh = fopen($rejects_file, "w")  or die("Return code: 10 Unable to create rejects file!" .$rejects_file . "\r\n");
            chmod($rejects_file, 0664);
        }
        else {
            $coaches = file($rejects_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
            foreach ($coaches as $line) {
                if ($line == $coach) {
                    echo "Return code 11: Coach reject answer already placed.\r\n";
                    return;
                }
            }
            $fh = fopen($rejects_file, "a+")  or die("Return code: 10 Unable to open accepts file to append new anwer!" .$rejects_file . "\r\n");
        }
        fwrite($fh, $coach . "\n");
        fclose($fh);
        echo "Return code: 0 Coach's answer to client's request REJECTED";
        return true;
    }
    echo "Return code: 11 Could not place coach's answer to client";
    return false;
}

function delete_coach_answer($client, $coach) {
    $answers = "";
    global $rootdir;
    $accepts_file = $rootdir . $client . "/coaches_accepted.txt";
    if (file_exists($accepts_file)) {
        $coaches = file($accepts_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
        foreach ($coaches as $line) {
            if ($line != $coach."AOK ")
                $answers = $answers . $line . "\r\n";
        }
        $fh = fopen($accepts_file, "w") or die("Return code: 10 Unable to open requests file!" .$accepts_file . "\r\n");
        fwrite($fh, $answers);
        fclose($fh);

        $rejects_file = $rootdir . $client . "/coaches_rejected.txt";
        if (file_exists($rejects_file)) {
            $answers = "";
            $coaches = file($rejects_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
            foreach ($coaches as $line) {
                if ($line != $coach."NAY ")
                    $answers = $answers . $line . "\r\n";
            }
            echo "Return code: 0 Coache's answer removed from the client's answers files.\r\n";
            return 0;
        }
    }
    echo "Return code: 12 Client's coach answer files do not exist";
    return false;
}

function list_coaches_answers($client) {
    $answer = "";
    global $rootdir;

    $accepts_file = $rootdir . $client . "/coaches_accepted.txt";
    if (file_exists($accepts_file)) {
        $coaches = file($accepts_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
        foreach ($coaches as $coach) {
            $answer = $answer.$coach."AOK ";
        }
    }

    $rejects_file = $rootdir . $client . "/coaches_rejected.txt";
    if (file_exists($rejects_file)) {
        $coaches = file($rejects_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
        foreach ($coaches as $coach) {
            $answer = $answer.$coach."NAY ";
        }
    }
    if ($answer != "")
        echo "Return code: 0 ".$answer;
    else
        echo "Return code: 1 No coaches's answers";
}

function accept_coach_answer($client, $coach)
{
    if (delete_coach_answer($client, $coach)) {
        global $rootdir;
        $clients_file = $rootdir . $coach . "/clients.txt";
        if (!file_exists($clients_file)) {
            $fh = fopen($clients_file, "w")  or die("Return code: 10 Unable to create coach's clients file! " .$clients_file . "\r\n");
            chmod($clients_file, 0664);
        }
        else {
            $clients = file($clients_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
            foreach ($clients as $line) {
                if ($line == $client) {
                    echo "Return code 11: ".$client." is already a client of ".$coach."\r\n";
                    return;
                }
            }
            $fh = fopen($clients_file, "a+")  or die("Return code: 10 Unable to open coach's clients file for appending! " .$clients_file . "\r\n");
        }
        fwrite($fh, $client . "\n");
        fclose($fh);

        $coaches_file = $rootdir . $client . "/coaches.txt";
        if (!file_exists($coaches_file)) {
            $fh = fopen($coaches_file, "w")  or die("Return code: 10 Unable to create client's coaches file! " .$coaches_file . "\r\n");
            chmod($coaches_file, 0664);
        }
        else {
            $coaches = file($coaches_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
            foreach ($coaches as $line) {
                if ($line == $coach) {
                    echo "Return code 11: ".$client." is already a client of ".$coach."\r\n";
                    return;
                }
            }
            $fh = fopen($coaches_file, "a+")  or die("Return code: 10 Unable to open client's coaches file for appending! " .$coaches_file . "\r\n");
        }
        fwrite($fh, $coach . "\n");
        fclose($fh);
        echo "Return code: 0 ".$client." is now a client of ".$coach;
        return true;
    }
    echo "Return code: 11 Could not make ".$client." a client of ".$coach;
    return false;
}

function reject_coach_answer($client, $coach)
{
    return delete_coach_answer($client, $coach);
}

function get_clients_list($coach) {
    global $rootdir;
    $clients_file = $rootdir . $coach . "/clients.txt";
    if (file_exists($clients_file)) {
        echo "Return code: 0 ";
        $clients = file($clients_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
        foreach ($clients as $client) {
            echo $client . " ";
        }
    }
    else
        echo "Return code: 12 Clients file does not exist" . $clients_file;
}

function remove_client_from_clients($coach, $client) {
    global $rootdir;
    $clients_file = $rootdir . $coach . "/clients.txt";

    if (file_exists($clients_file)) {
        $clients = file($clients_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
        foreach ($clients as $line) {
            if ($line != $client) {
                $cur_clients = $cur_clients . $line . "\r\n";
            }
        }
        $fh = fopen($clients_file, "w") or die("Return code: 10 Unable to open coaches clients file!" .$clients_file . "\r\n");
        fwrite($fh, $cur_clients);
        fclose($fh);
        echo "Return code: 0 Client removed from the coach's clients file.\r\n";
        return true;
    }
}

function get_coaches_list($client) {
    global $rootdir;
    $coaches_file = $rootdir . $client . "/coaches.txt";
    if (file_exists($coaches_file)) {
        echo "Return code: 0 ";
        $coaches = file($coaches_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
        foreach ($coaches as $coach) {
            echo $coach . " ";
        }
    }
    else
        echo "Return code: 12 Coaches file does not exist" . $coaches_file;
}

function remove_coach_from_coaches($client, $coach) {
    global $rootdir;
    $coaches_file = $rootdir . $client . "/coaches.txt";

    if (file_exists($coaches_file)) {
        $coaches = file($coaches_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
        foreach ($coaches as $line) {
            if ($line != $coach) {
                $cur_coaches = $cur_coaches . $line . "\r\n";
            }
        }
        $fh = fopen($coaches_file, "w") or die("Return code: 10 Unable to open client's coaches file!" .$coaches_file . "\r\n");
        fwrite($fh, $cur_coaches);
        fclose($fh);
        echo "Return code: 0 Coach removed from the client's file.\r\n";
        return true;
    }
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

function update_datafile_with_password($userid) {
    global $rootdir;
    global $htpasswd_file;

    $users = file($htpasswd_file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
    $password="";
    foreach ($users as $user) {
        $sep=strpos($user, ':');
        $username=substr($user, 0, $sep);
        if ($username == $userid) {
            $password=substr($user, $sep+1, strlen($user)-$sep);
            break;
        }
    }
    if ($password != "") {
        $userdatafile = $rootdir . $userid . "/user.data";
        if (file_exists($userdatafile)) {
            $fh = fopen($userdatafile, 'a')  or die("Return code: 10 Unable to open user.data file!" .$userdatafile . "\r\n");
            fwrite($fh, $password);
            fclose($fh);
            return true;
        }
    }
    return false;
}

function run_dbscript($cmd, $cmd_opt, $userid, $print_output) {
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
    if ($print_output)
        echo "Return code: " . $return_var . "  " . $output;
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
            if (isset($_GET['deleteclientrequest'])) {
                $client = $_GET['deleteclientrequest'];
                if ($client)
                    delete_client_request($username, $client);
                exit;
            }
            if (isset($_GET['listclientsrequests'])) {
                list_clients_requests($username);
                exit;
            }
            if (isset($_GET['acceptclientrequest'])) {
                $client = $_GET['acceptclientrequest'];
                if ($client)
                    accept_client_request($username, $client);
                exit;
            }
            if (isset($_GET['rejectclientrequest'])) {
                $client = $_GET['rejecttclientrequest'];
                if ($client)
                    reject_client_request($username, $client);
                exit;
            }

            if (isset($_GET['deletecoachanswer'])) {
                $coach = $_GET['deletecoachanswer'];
                if ($coach)
                    delete_coach_answer($username, $coach);
                exit;
            }
            if (isset($_GET['listcoachesanswers'])) {
                list_coaches_answers($username);
                exit;
            }
            if (isset($_GET['acceptcoachanswer'])) {
                $coach = $_GET['acceptcoachanswer'];
                if ($coach)
                    accept_coach_answer($username, $coach);
                exit;
            }
            if (isset($_GET['rejectcoachanswer'])) {
                $coach = $_GET['rejectcoachanswer'];
                if ($coach)
                    reject_coach_answer($username, $coach);
                exit;
            }

            if (isset($_GET['getclients'])) {
                get_clients_list($username);
                exit;
            }
            if (isset($_GET['removecurclient'])) {
                $client = $_GET['removecurclient'];
                if ($client)
                    remove_client_from_clients($username, $client);
                exit;
            }

            if (isset($_GET['getcoaches'])) {
                get_coaches_list($username);
                exit;
            }
            if (isset($_GET['removecurcoach'])) {
                $coach = $_GET['removecurcoach'];
                if ($coach)
                    remove_coach_from_coaches($username, $coach);
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
                download_file($_GET['file'], $filedir);
                exit;
            }
            if (isset($_GET['getbinfile'])) {
                $binfile = $_GET['getbinfile'];
                if ($binfile) {
                    $targetuser = isset($_GET['fromuser']) ? $_GET['fromuser'] : $username;
                    get_binfile($binfile, $targetuser);
                    exit;
                }
            }

            //?user=1739556367374&password=lrTp1$&getbinfile=resume&fromuser=1739556367374
            if (isset($_GET['checkfilectime'])) {
                $binfile = $_GET['checkfilectime'];
                if ($binfile) {
                    $targetuser = isset($_GET['fromuser']) ? $_GET['fromuser'] : '';
                    if ($targetuser) {
                        check_file_ctime($binfile, $targetuser);
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
        $password = isset($_GET['password']) ? $_GET['password'] : '';
        run_dbscript("getid", $query . ' ' . $password , "", true);
        exit;
    }

    $userid = isset($_GET['onlinedata']) ? $_GET['onlinedata'] : '';
    if ($userid) { //Check if there is an already existing user in the online database. The  unique key used to identify an user is decided on the TrainingPlanner app source code. This script is agnostic to it
        run_dbscript("getall", "", $userid, true);
        exit;
    }

    $userid = isset($_GET['alteronlineuser']) ? $_GET['alteronlineuser'] : '';
    if ($userid) { //dbscript expects the file user.data to have been previously uploaded to $userid dir
        if (update_datafile_with_password($userid))
            run_dbscript("add", "", $userid, true);
        exit;
    }

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
        $ok = run_htpasswd("-bB", $username, $new_user_password);
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
            run_dbscript("del", "", $username, false);
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
            $ok = run_htpasswd("-bB", $new_username, $new_password);
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
