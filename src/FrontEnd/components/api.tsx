import { Button } from "@/components/ui/button"
import { useState, useEffect } from 'react'
import { Spinner } from "@/components/ui/spinner"

function Api_call(){

    const [message, setMessage] = useState('');
    const [name, setName] = useState('');
    const [response, setResponse] = useState('');
    const [loading, setLoading] = useState(false);

    // Fetch on component mount
    useEffect(() => {
        fetchHello();
    }, []);

    const fetchHello = async () => {
        setLoading(true);
        try {
            const res = await fetch('http://localhost:8080/api/hello');
            const text = await res.text(); // Your C++ server returns plain text, not JSON
            setMessage(text);
        } catch (error) {
            console.error('Error fetching:', error);
            setMessage('Error connecting to C++ server');
        } finally {
            setLoading(false);
        }
    };

    const sendData = async () => {
        if (!name.trim()) {
            alert('Please enter a name');
            return;
        }

        setLoading(true);
        try {
            const res = await fetch('http://localhost:8080/api/data', {
                method: 'POST',
                headers: {
                    'Content-Type': 'text/plain',
                },
                body: name
            });
            const text = await res.text();
            setResponse(text);
        } catch (error) {
            console.error('Error posting:', error);
            setResponse('Error sending data to C++ server');
        } finally {
            setLoading(false);
        }
    };

    return (
    <div className="flex min-h-svh flex-col items-center justify-center gap-6 p-4">
        <div className="flex flex-col items-center gap-4 w-full max-w-md">

            {/* GET */}
            <div className="flex flex-col items-center gap-2 w-full">
                <Button
                    onClick={fetchHello}
                    disabled={loading}
                    className="w-full"
                >
                    {loading ? <Spinner /> : 'Test Connection'}
                </Button>
                {message && (
                    <div className="p-4 border rounded-md w-full">
                        <p className="text-sm font-semibold">Response:</p>
                        <p>{message}</p>
                    </div>
                )}
            </div>

            {/* POST */}
            <div className="flex flex-col gap-2 w-full">
                <input
                    type="text"
                    value={name}
                    onChange={(e) => setName(e.target.value)}
                    placeholder="Enter Token here!"
                    className="px-4 py-2 border rounded-md"
                />
                <Button
                    onClick={sendData}
                    disabled={loading}
                    variant="secondary"
                    className="w-full"
                >
                    {loading ? <Spinner /> : 'Request to server'}
                </Button>
                {response && (
                    <div className="p-4 border rounded-md w-full">
                        <p className="text-sm font-semibold">Server Response:</p>
                        <p>{response}</p>
                    </div>
                )}
            </div>
        </div>
    </div>

    )
}
export default Api_call