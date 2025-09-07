
PORT = 8002

print(f"🧊 Server running on: http://localhost:{PORT}/")
# print("  - Should respond to GET / with status 200 and HTML content")

import http.client as httplib

def test_root():
    """ expected config:
server {
	listen PORT;
	root www;
	index index.html;
    
	location / {
		root wowow/www;
		index index.html;
		methods GET;
		autoindex off;
	}
"""
    conn = httplib.HTTPConnection("localhost", PORT, timeout=5)
    try:
        conn.request("GET", "/")
        resp = conn.getresponse()
        print("Status:", resp.status)
        print("Reason:", resp.reason)
        content = resp.read()
        print("Content-Type:", resp.getheader("Content-Type"))
        print("Body (first 200 chars):", content[:200])
        assert resp.status == 200, "Expected status 200"
        assert "text/html" in resp.getheader("Content-Type", ""), "Expected HTML content"
        print("Test PASSED")
    except Exception as e:
        print("Test FAILED:", e)
    finally:
        conn.close()

# test allowed methods
def test_allowed_methods():
    """
    having:
    location / {
            methods GET POST HEAD;
        }
    should allow GET and POST, but not DELETE or PUT, HEAD is not supported by server
    200 for GET and POST
    405 for DELETE and PUT
    501 for HEAD
    """
    conn = httplib.HTTPConnection("localhost", PORT, timeout=5)
    methods = ["GET", "POST", "DELETE", "PUT", "HEAD"]
    expected_status = {"GET": 200, "POST": 200, "DELETE": 405, "PUT": 405, "HEAD": 501}
    for method in methods:
        try:
            conn.request(method, "/")
            resp = conn.getresponse()
            print(f"Method: {method}, Status: {resp.status}, Reason: {resp.reason}")
            assert resp.status == expected_status[method], f"Expected status {expected_status[method]} for method {method}"
        except Exception as e:
            print(f"❌ Test FAILED for method {method}:", e)
        finally:
            conn.close()
    print("Allowed methods test completed")



if __name__ == "__main__":
    #test_root()
    test_allowed_methods()