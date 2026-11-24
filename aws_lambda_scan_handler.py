import json
import time
import os
import boto3

ITEMS_TABLE = os.getenv("ITEMS_TABLE", "items")
TRANSACTIONS_TABLE = os.getenv("TRANSACTIONS_TABLE", "transactions")
ALERTS_TABLE = os.getenv("ALERTS_TABLE", "alerts")
SECRET_KEY = os.getenv("API_KEY", "<DEVICE_SECRET>")

dynamodb = boto3.resource("dynamodb")
items_table = dynamodb.Table(ITEMS_TABLE)
transactions_table = dynamodb.Table(TRANSACTIONS_TABLE)
alerts_table = dynamodb.Table(ALERTS_TABLE)


def lambda_handler(event, context):
    qs = event.get("queryStringParameters") or {}
    tag_id = qs.get("tag")
    device = qs.get("dev", "GATE-01")
    api_key = qs.get("key")
    action = (qs.get("action") or "IN").upper()

    if not tag_id or api_key != SECRET_KEY:
        alerts_table.put_item(
            Item={
                "device_id": device,
                "timestamp": int(time.time()),
                "tag_id": tag_id or "UNKNOWN",
                "reason": "Invalid Key or Malformed Request",
            }
        )
        return {"statusCode": 403, "body": "#UNAUTH#"}

    resp = items_table.get_item(Key={"tag_id": tag_id})
    item = resp.get("Item")
    ts = int(time.time())

    if not item or not item.get("authorised", False):
        alerts_table.put_item(
            Item={
                "device_id": device,
                "timestamp": ts,
                "tag_id": tag_id,
                "reason": "Unauthorized Tag Attempt",
            }
        )
        return {"statusCode": 200, "body": "#UNAUTH#"}

    qty = 1
    delta = qty if action == "IN" else -qty

    items_table.update_item(
        Key={"tag_id": tag_id},
        UpdateExpression=(
            "SET current_stock = if_not_exists(current_stock, :zero) + :delta, "
            "last_scan_ts = :ts"
        ),
        ExpressionAttributeValues={
            ":zero": 0,
            ":delta": delta,
            ":ts": ts,
        },
    )

    transactions_table.put_item(
        Item={
            "item_id": item["item_id"],
            "timestamp": ts,
            "movement": action,
            "quantity": qty,
            "batch_id": item["batch_id"],
            "device_id": device,
        }
    )

    return {
        "statusCode": 200,
        "headers": {"Access-Control-Allow-Origin": "*"},
        "body": f"#{action},{item['item_id']},{qty}#",
    }
