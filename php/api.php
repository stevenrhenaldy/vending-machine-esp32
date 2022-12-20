<?php
ini_set('display_errors', 1);
ini_set('display_startup_errors', 1);
error_reporting(E_ALL);

include("dbconnect.php");

if(isset($_GET['action'], $_POST['secret'], $_POST['vending_id'])){

    $action = $_GET['action'];
    $vending_id = $_POST['vending_id'];
    $secret = $_POST['secret'];
    switch($action){
        case "sync":
            sync($conn, $vending_id, $secret);
            break;
        case "payment":
            if(isset($_POST['card_uid'], $_POST['item_num'])){
                $card_uid = $_POST['card_uid'];
                $item_num = $_POST['item_num'];
                payment($conn, $vending_id, $secret, $card_uid, $item_num);
            }else{
                echo json_encode([
                    "status" => "error",
                    "message" => "Invalid Data!",
                ]);
                return;
            }
            break;
        
    }
}else{
    echo json_encode([
        "status" => "error",
        "message" => "Invalid Data!",
    ]);
    return;
}

function payment($conn, $vending_id, $secret, $card_uid, $item_num){
    if(auth($conn, $vending_id, $secret) == NULL){
        return;
    }

    $item = getItem($conn, $vending_id, $item_num);
    if(!$item){
        echo json_encode([
            "status" => "error",
            "message" => "Invalid",
        ]);
        return;
    }
    if($item['quantity'] <= 0){
        echo json_encode([
            "status" => "error",
            "message" => "Out of Stock",
        ]);
        return;
    }

    cutBalance($conn, $vending_id, $card_uid, $item);
    return;
}

function sync($conn, $vending_id, $secret){
    if(auth($conn, $vending_id, $secret) == NULL){
        return;
    }

    $stmt = $conn->prepare("SELECT * FROM vending_items_view WHERE vending_id=?");
    $stmt->execute([$vending_id]); 
    $items = $stmt->fetchAll();
    $data = array();
    foreach( $items as $item ){
        $data[$item['num']] = [
            "name" => $item['name'],
            "price" => $item['price'],
            "quantity" => $item['quantity']
        ];
    }

    echo json_encode($data);
    return;
}


function getItem($conn, $vending_id, $item_num){
    $stmt = $conn->prepare("SELECT * FROM vending_items_view WHERE vending_id=? AND num=?");
    $stmt->execute([$vending_id, $item_num]); 
    $items = $stmt->fetchAll();
    $item = $items[0];
    return $item;
}

function cutBalance($conn, $vending_id, $card_uid, $item){
    $stmt = $conn->prepare("SELECT * FROM cards WHERE card_data=?");
    $stmt->execute([$card_uid]); 
    $cards = $stmt->fetchAll();
    if(!$cards){
        echo json_encode([
            "status" => "error",
            "message" => "Card not registered",
        ]);
        return;
    }
    $card = $cards[0];
    $card_balance = $card['balance'];

    if($card_balance < $item['price']){
        echo json_encode([
            "status" => "error",
            "message" => "Top up Balance",
        ]);
        return;
    }

    $stmt2 = $conn->prepare("UPDATE `vending_items` SET `quantity` = ? WHERE `vending_id` = ? AND `item_id` = ?");
    $stmt2->execute([$item['quantity']-1, $vending_id, $item['id']]); 
    

    $card_balance -= $item['price'];
    $stmt3 = $conn->prepare("UPDATE `cards` SET `balance` = ? WHERE `cards`.`id` = ?");
    $stmt3->execute([$card_balance, $card['id']]); 

    $stmt4 = $conn->prepare("INSERT INTO `transactions` (`card_id`, `vending_id`, `item_id`, `price`) VALUES (?, ?, ?, ?)");
    $stmt4->execute([$card['id'], $vending_id, $item['id'], $item['price']]); 
    echo json_encode([
        "status" => "success",
        "balance" => $card_balance,
        "message" => "Payment success!",
    ]);
    return;
}



function auth($conn, $vending_id, $secret){
    $stmt = $conn->prepare("SELECT * FROM vendings WHERE id=?");
    $stmt->execute([$vending_id]);
    $vendings = $stmt->fetchAll();
    if(!$vendings){
        echo json_encode([
            "status" => "error",
            "message" => "Vending not found",
        ]);
        return null;
    }
    $vending = $vendings[0];
    if($vending['secret_key'] != $secret){
        echo json_encode([
            "status" => "error",
            "message" => "Authentication failed",
        ]);
        return NULL;
    }
    return $vending;
}



?>
