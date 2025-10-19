import { Button } from "@/components/ui/button"
import { useState, useEffect } from 'react'
import { Spinner } from "@/components/ui/spinner"
import {
  Card,
  CardAction,
  CardContent,
  CardDescription,
  CardFooter,
  CardHeader,
  CardTitle,
} from "@/components/ui/card"
import { ScrollArea } from "@/components/ui/scroll-area"

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
            alert('Enter data');
            return;
        }

        setLoading(true);
        try {
            const res = await fetch('http://localhost:8080/api/find_classes', {
                method: 'POST',
                headers: {
                    'Content-Type': 'text/plain',
                },
                body: name
            });
            const text = await res.text();
            const parsedData = JSON.parse(text)
            setResponse(text);
        } catch (error) {
            console.error('Error posting:', error);
            setResponse('Error sending data to C++ server');
        } finally {
            setLoading(false);
        }
    };

    return (
    <div className="flex min-h-svh flex-col items-center justify-center ">


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
                    {loading ? <Spinner /> : 'Send Request'}
                </Button>
                {response && (
                
                        
                   

                        <ScrollArea className="h-[600px] w-full rounded-md border">
                            <div className="flex flex-col space-y-4 p-4">
                                {JSON.parse(response).map((card) => (
                                card.course_name && (<Card key={card.id}>
                                    <CardHeader>
                                    <CardTitle>{card.course_name}</CardTitle>
                                    <CardDescription>Created at: {card.created_at} </CardDescription>
                                    </CardHeader>
                                    <CardContent>
                                    <p>{card.name}  {card.due_at && ` Due at: ${card.due_at}`}</p>
                                    </CardContent>
                                </Card>)
                                ))}
                            </div>
                        </ScrollArea>

                )}
            </div>
        
    </div>

    )
}
export default Api_call